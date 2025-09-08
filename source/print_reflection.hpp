#pragma once

#include <optional>
#include <string>

#include <slang.h>

namespace print_reflection
{
struct PrintBufferReflection
{
    uint32_t binding = 0;    // binding index for the print buffer
    uint32_t space = 0;    // descriptor set / register space
};

/**
 * Reflect binding information for the global gPrintBuffer parameter.
 * @param program linked Slang program
 * @param name name of the print buffer variable (default gPrintBuffer)
 * @return reflection information if found
 */
[[nodiscard]] std::optional<PrintBufferReflection> ReflectPrintBuffer(
    slang::IComponentType* program, const std::string& name = "gPrintBuffer");

}    // namespace print_reflection
