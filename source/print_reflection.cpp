#include "print_reflection.hpp"

#include <slang-com-ptr.h>
#include <slang.h>

namespace print_reflection
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

std::optional<PrintBufferReflection> ReflectPrintBuffer(
    slang::IComponentType* program, const std::string& name)
{
    if (!program) {
        return std::nullopt;
    }

    slang::ProgramLayout* layout = program->getLayout();
    if (!layout) {
        return std::nullopt;
    }

    slang::VariableLayoutReflection* global =
        layout->getGlobalParamsVarLayout();
    if (!global) {
        return std::nullopt;
    }

    slang::VariableLayoutReflection* scope = global;
    auto* typeLayout = scope->getTypeLayout();

    if (typeLayout
        && (typeLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock
            || typeLayout->getKind()
                == slang::TypeReflection::Kind::ConstantBuffer))
    {
        scope = typeLayout->getElementVarLayout();
        typeLayout = scope ? scope->getTypeLayout() : nullptr;
    }

    if (!scope || !typeLayout
        || typeLayout->getKind() != slang::TypeReflection::Kind::Struct)
    {
        return std::nullopt;
    }

    slang::VariableLayoutReflection* printVar = FindField(typeLayout, name);
    if (!printVar) {
        return std::nullopt;
    }

    PrintBufferReflection result {};
    result.binding = static_cast<uint32_t>(
        scope->getOffset(slang::ParameterCategory::DescriptorTableSlot)
        + printVar->getOffset(slang::ParameterCategory::DescriptorTableSlot));
    result.space = static_cast<uint32_t>(
        scope->getBindingSpace(slang::ParameterCategory::DescriptorTableSlot)
        + printVar->getBindingSpace(
            slang::ParameterCategory::DescriptorTableSlot));

    return result;
}

}    // namespace print_reflection
