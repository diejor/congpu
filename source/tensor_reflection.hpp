#pragma once
#include <string>
#include <vector>

#include <slang.h>

namespace tensor_reflection
{

/// Describes the binding location of a resource or constant buffer.
struct BindingInfo
{
    slang::ParameterCategory category = slang::ParameterCategory::None;
    int index = 0;
    int space = 0;
};

/// Reflection information for a TensorBuffer parameter.
struct TensorBufferInfo
{
    std::string name;            ///< Name of the parameter
    BindingInfo data;            ///< Binding for the underlying StructuredBuffer/RWStructuredBuffer
    BindingInfo shapeBuffer;     ///< Binding for the constant buffer holding the shape information
    size_t      shapeOffset = 0; ///< Byte offset of the shape structure inside the constant buffer
};

/// Reflect all TensorBuffer parameters in the given program layout.
[[nodiscard]]
std::vector<TensorBufferInfo> reflectTensorBuffers(slang::ProgramLayout* programLayout);

} // namespace tensor_reflection

