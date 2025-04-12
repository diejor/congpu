#pragma once

#include <string>
#include <vector>
#include "result.h"
#include <slang.h>

namespace slang_compiler {

/**
 * @brief Compiles a Slang module (provided as a source string) into WGSL.
 *
 * This function creates a Slang compilation session, loads the provided Slang 
 * source code, adds the given entry points, links the module, and then generates 
 * WGSL output code. All operations are performed in memory.
 *
 * @param moduleName         A name for the module (used for diagnostics).
 * @param slangSource        The complete Slang source code as a string.
 * @param entryPoints        A vector containing the entry point names to compile.
 * @param includeDirectories An optional vector of directory paths for additional 
 *                           include search paths.
 * @return Result<std::string, Error> On success, returns the generated WGSL code 
 *         as a string; otherwise returns an Error.
 */
Result<std::string, Error> compileSlangToWgsl(
    const std::string& moduleName,
    const std::string& slangSource,
    const std::vector<std::string>& entryPoints,
    const std::vector<std::string>& includeDirectories = {}
);

} // namespace slang_compiler

