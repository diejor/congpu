#pragma once

#include <optional>
#include <string>

#include <slang.h>

namespace tensor_reflection
{
struct TensorBufferReflection
{
    uint32_t dataBinding = 0;    // binding index for RWStructuredBuffer data
    uint32_t dataSpace = 0;    // descriptor set / register space for data
    uint32_t shapeBinding =
        0;    // binding index for constant buffer storing shape
    uint32_t shapeSpace = 0;    // descriptor set / register space for shape
    size_t shapeOffset = 0;    // byte offset of shape struct in constant buffer
    size_t shapeSize = 0;    // size in bytes of shape struct
};

/**
 * Reflect binding information for a tensor buffer parameter.
 * @param program linked Slang program
 * @param paramName name of the tensor buffer parameter to reflect
 * @return reflection information if found
 */
[[nodiscard]]
std::optional<TensorBufferReflection> ReflectTensorBuffer(
    slang::IComponentType* program, const std::string& paramName);

}    // namespace tensor_reflection
