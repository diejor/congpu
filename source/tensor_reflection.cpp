#include "tensor_reflection.hpp"

#include <slang-com-ptr.h>
#include <slang.h>

namespace tensor_reflection
{
namespace
{
// Helper to locate a field by name inside a struct type layout
slang::VariableLayoutReflection* FindField(
    slang::TypeLayoutReflection* typeLayout, const std::string& name)
{
    if (!typeLayout
        || typeLayout->getKind() != slang::TypeReflection::Kind::Struct)
    {
        return nullptr;
    }
    const int fieldCount = typeLayout->getFieldCount();
    for (int i = 0; i < fieldCount; ++i) {
        slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(i);
        if (name == field->getName()) {
            return field;
        }
    }
    return nullptr;
}

}    // namespace

std::optional<TensorBufferReflection> ReflectTensorBuffer(
    slang::IComponentType* program, const std::string& paramName)
{
    if (!program) {
        return std::nullopt;
    }

    slang::ProgramLayout* layout = program->getLayout();
    if (!layout) {
        return std::nullopt;
    }

    // Global scope of the program
    slang::VariableLayoutReflection* global =
        layout->getGlobalParamsVarLayout();
    if (!global) {
        return std::nullopt;
    }

    // Global scope may be wrapped in a parameter block and constant buffer.
    slang::VariableLayoutReflection* scope = global;
    auto* typeLayout = scope->getTypeLayout();

    TensorBufferReflection result {};

    if (typeLayout
        && (typeLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock
            || typeLayout->getKind()
                == slang::TypeReflection::Kind::ConstantBuffer))
    {
        // Record binding information for the automatically introduced constant
        // buffer
        slang::VariableLayoutReflection* container =
            typeLayout->getContainerVarLayout();
        if (container) {
            result.shapeBinding = static_cast<uint32_t>(container->getOffset(
                slang::ParameterCategory::DescriptorTableSlot));
            result.shapeSpace =
                static_cast<uint32_t>(container->getBindingSpace(
                    slang::ParameterCategory::DescriptorTableSlot));
        }
        scope = typeLayout->getElementVarLayout();
        typeLayout = scope ? scope->getTypeLayout() : nullptr;
    }

    if (!scope || !typeLayout
        || typeLayout->getKind() != slang::TypeReflection::Kind::Struct)
    {
        return std::nullopt;
    }

    slang::VariableLayoutReflection* tensorVar =
        FindField(typeLayout, paramName);
    if (!tensorVar) {
        return std::nullopt;
    }

    // Access fields of the tensor buffer
    slang::TypeLayoutReflection* tensorType = tensorVar->getTypeLayout();
    slang::VariableLayoutReflection* dataField = FindField(tensorType, "data");
    slang::VariableLayoutReflection* shapeField =
        FindField(tensorType, "shape");
    if (!dataField || !shapeField) {
        return std::nullopt;
    }

    // Compute cumulative binding for the data field
    result.dataBinding = static_cast<uint32_t>(
        scope->getOffset(slang::ParameterCategory::DescriptorTableSlot)
        + tensorVar->getOffset(slang::ParameterCategory::DescriptorTableSlot)
        + dataField->getOffset(slang::ParameterCategory::DescriptorTableSlot));
    result.dataSpace = static_cast<uint32_t>(
        scope->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot)
        + tensorVar->getBindingSpace(
            slang::ParameterCategory::DescriptorTableSlot)
        + dataField->getBindingSpace(
            slang::ParameterCategory::DescriptorTableSlot));

    // Compute offset and size for the shape struct within the constant buffer
    result.shapeOffset = scope->getOffset(slang::ParameterCategory::Uniform)
        + tensorVar->getOffset(slang::ParameterCategory::Uniform)
        + shapeField->getOffset(slang::ParameterCategory::Uniform);
    if (auto* shapeType = shapeField->getTypeLayout()) {
        result.shapeSize =
            shapeType->getSize(slang::ParameterCategory::Uniform);
    }

    return result;
}

}    // namespace tensor_reflection
