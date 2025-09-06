#include <cstring>
#include <string_view>

#include "tensor_reflection.hpp"

namespace tensor_reflection
{
namespace
{
using InfoVec = std::vector<TensorBufferInfo>;

BindingInfo getBinding(slang::VariableLayoutReflection* var)
{
    BindingInfo result;
    const int categoryCount = var->getCategoryCount();
    if (categoryCount == 0) {
        return result;
    }

    const auto cat = var->getCategoryByIndex(0);
    result.category = cat;
    result.index = static_cast<int>(var->getOffset(cat));
    result.space = static_cast<int>(var->getBindingSpace(cat));
    return result;
}

BindingInfo getNonUniformBinding(slang::VariableLayoutReflection* var)
{
    BindingInfo result;
    const int categoryCount = var->getCategoryCount();
    for (int i = 0; i < categoryCount; ++i) {
        const auto cat = var->getCategoryByIndex(i);
        if (cat == slang::ParameterCategory::Uniform) {
            continue;
        }
        result.category = cat;
        result.index = static_cast<int>(var->getOffset(cat));
        result.space = static_cast<int>(var->getBindingSpace(cat));
        break;
    }
    return result;
}

bool isTensorBufferType(slang::TypeLayoutReflection* typeLayout)
{
    if (!typeLayout) {
        return false;
    }
    const char* name = typeLayout->getName();
    if (!name) {
        return false;
    }
    std::string_view n(name);
    return n.rfind("TensorBuffer", 0) == 0 || n.rfind("RWTensorBuffer", 0) == 0;
}

void reflectTensorBuffer(slang::VariableLayoutReflection* varLayout,
                         InfoVec& out)
{
    TensorBufferInfo info;
    info.name = varLayout->getName() ? varLayout->getName() : "";
    info.data = getNonUniformBinding(varLayout);

    auto* elementType = varLayout->getTypeLayout();
    const int fieldCount = elementType->getFieldCount();
    for (int i = 0; i < fieldCount; ++i) {
        auto* field = elementType->getFieldByIndex(i);
        const char* fname = field->getName();
        if (!fname) {
            continue;
        }
        if (std::strcmp(fname, "shape") == 0) {
            if (auto* shapeContainer =
                    field->getTypeLayout()->getContainerVarLayout()) {
                info.shapeBuffer = getBinding(shapeContainer);
            }
            info.shapeOffset =
                field->getOffset(slang::ParameterCategory::Uniform);
        }
    }

    out.push_back(info);
}

void traverse(slang::VariableLayoutReflection* varLayout, InfoVec& out)
{
    auto* typeLayout = varLayout->getTypeLayout();
    if (isTensorBufferType(typeLayout)) {
        reflectTensorBuffer(varLayout, out);
        return;
    }
    switch (typeLayout->getKind()) {
        case slang::TypeReflection::Kind::Struct: {
            const int fieldCount = typeLayout->getFieldCount();
            for (int i = 0; i < fieldCount; ++i) {
                traverse(typeLayout->getFieldByIndex(i), out);
            }
            break;
        }
        case slang::TypeReflection::Kind::ParameterBlock:
        case slang::TypeReflection::Kind::ConstantBuffer:
        case slang::TypeReflection::Kind::TextureBuffer:
        case slang::TypeReflection::Kind::ShaderStorageBuffer: {
            auto* elementVar = typeLayout->getElementVarLayout();
            traverse(elementVar, out);
            break;
        }
        default:
            break;
    }
}

}    // namespace

std::vector<TensorBufferInfo> reflectTensorBuffers(slang::ProgramLayout* layout)
{
    InfoVec result;
    if (!layout) {
        return result;
    }

    traverse(layout->getGlobalParamsVarLayout(), result);
    const int entryCount = layout->getEntryPointCount();
    for (int i = 0; i < entryCount; ++i) {
        traverse(layout->getEntryPointByIndex(i)->getVarLayout(), result);
    }
    return result;
}

}    // namespace tensor_reflection
