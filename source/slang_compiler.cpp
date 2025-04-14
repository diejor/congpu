#include <algorithm>
#include <iostream>
#include <vector>

#include "slang_compiler.hpp"

#include <slang-com-ptr.h>
#include <slang.h>

using namespace slang_compiler;

/**
 * @brief Structure to hold the active Slang session.
 */
struct SessionInfo
{
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    Slang::ComPtr<slang::ISession> session;
};

/**
 * @brief Creates a new Slang session with optional include directories.
 *
 * @param includeDirectories A vector of include directory paths.
 * @return Result<SessionInfo, Error> Returns a SessionInfo on success,
 * otherwise an Error.
 */
Result<SessionInfo, Error> createSlangSession(
    const std::vector<std::string>& includeDirectories)
{
    SessionInfo sessionInfo;
    slang::createGlobalSession(sessionInfo.globalSession.writeRef());

    slang::SessionDesc sessionDesc;

    // Set up a single target for WGSL output.
    slang::TargetDesc target;
    target.format = SLANG_WGSL;
    sessionDesc.targets = &target;
    sessionDesc.targetCount = 1;

    std::vector<const char*> includeDirectoriesData(includeDirectories.size());
    std::transform(includeDirectories.cbegin(),
                   includeDirectories.cend(),
                   includeDirectoriesData.begin(),
                   [](const std::string& path) { return path.c_str(); });
    sessionDesc.searchPaths = includeDirectoriesData.data();
    sessionDesc.searchPathCount =
        static_cast<SlangInt>(includeDirectoriesData.size());

    sessionInfo.globalSession->createSession(sessionDesc,
                                             sessionInfo.session.writeRef());

    return sessionInfo;
}

/**
 * @brief Structure to hold a loaded module and its dependency files.
 */
struct ModuleInfo
{
    Slang::ComPtr<slang::IComponentType>
        program;    ///< Handle to the composite component.
    std::vector<std::string>
        dependencyFiles;    ///< List of dependency file paths.
};

/**
 * @brief Loads a Slang module from a source string.
 *
 * @param session      The active Slang session.
 * @param moduleName   The name to give the module.
 * @param slangSource  The Slang source code.
 * @param entryPoints  Entry point names to add into the composite module.
 * @return Result<ModuleInfo, Error> Returns a ModuleInfo on success, otherwise
 * an Error.
 */
Result<ModuleInfo, Error> loadSlangModule(
    const Slang::ComPtr<slang::ISession>& session,
    const std::string& moduleName,
    const std::vector<std::string>& entryPoints)
{
    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module =
        session->loadModule(moduleName.c_str(), diagnostics.writeRef());
    std::cout << "Loaded module '" << moduleName << "'\n";
    if (diagnostics) {
        std::string message =
            reinterpret_cast<const char*>(diagnostics->getBufferPointer());
        return Error {"Could not load slang module '" + moduleName + "':\n"
                      + message};
    }
    std::cout << "Loaded module, diagnostics: '" << diagnostics << "'\n";

    // Gather dependency files if any.
    size_t depCount = static_cast<size_t>(module->getDependencyFileCount());
    std::vector<std::string> dependencyFiles(depCount);
    for (size_t i = 0; i < depCount; ++i) {
        dependencyFiles[i] =
            module->getDependencyFilePath(static_cast<SlangInt32>(i));
    }

    // Compose the module by adding the entry points.
    std::vector<slang::IComponentType*> components;
    components.push_back(module);
    for (const std::string& entryPointName : entryPoints) {
        Slang::ComPtr<slang::IEntryPoint> entryPoint;
        SlangResult res = module->findEntryPointByName(entryPointName.c_str(),
                                                       entryPoint.writeRef());
        if (SLANG_FAILED(res)) {
            return Error {"Entrypoint '" + entryPointName
                          + "' not found in module '" + moduleName + "'."};
        }
        components.push_back(entryPoint);
    }
    Slang::ComPtr<slang::IComponentType> program;
    session->createCompositeComponentType(
        components.data(),
        static_cast<SlangInt32>(components.size()),
        program.writeRef());

    return ModuleInfo {program, dependencyFiles};
}

/**
 * @brief Compiles a loaded composite module to WGSL.
 *
 * This function links the module and extracts the WGSL code.
 *
 * @param program    The composite module.
 * @param moduleName The name of the module (for diagnostics).
 * @return Result<std::string, Error> Returns the WGSL code as a string on
 * success; otherwise, an Error.
 */
Result<std::string, Error> compileModuleToWgsl(
    const Slang::ComPtr<slang::IComponentType>& program,
    const std::string& moduleName)
{
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    Slang::ComPtr<ISlangBlob> linkDiagnostics;
    program->link(linkedProgram.writeRef(), linkDiagnostics.writeRef());
    if (linkDiagnostics) {
        std::string message =
            reinterpret_cast<const char*>(linkDiagnostics->getBufferPointer());
        return Error {"Could not link module '" + moduleName + "': " + message};
    }

    Slang::ComPtr<slang::IBlob> codeBlob;
    Slang::ComPtr<ISlangBlob> codeDiagnostics;
    int targetIndex = 0;
    linkedProgram->getTargetCode(
        targetIndex, codeBlob.writeRef(), codeDiagnostics.writeRef());
    // With a single target configured (WGSL)
    if (codeDiagnostics) {
        std::string message =
            reinterpret_cast<const char*>(codeDiagnostics->getBufferPointer());
        return Error {"Could not generate WGSL for module '" + moduleName
                      + "': " + message};
    }

    std::string wgslSource =
        reinterpret_cast<const char*>(codeBlob->getBufferPointer());
    return wgslSource;
}

Result<std::string, Error> slang_compiler::compileSlangToWgsl(
    const std::string& moduleName,
    const std::vector<std::string>& entryPoints,
    const std::vector<std::string>& includeDirectories)
{
    // Create a Slang session with the provided include directories.
    SessionInfo sessionInfo;
    TRY_ASSIGN(sessionInfo, createSlangSession(includeDirectories));

    // Load the module from the provided in-memory source.
    ModuleInfo moduleInfo;
    TRY_ASSIGN(moduleInfo,
               loadSlangModule(sessionInfo.session, moduleName, entryPoints));

    // Compile the loaded module into WGSL.
    std::string wgslOutput;
    TRY_ASSIGN(wgslOutput, compileModuleToWgsl(moduleInfo.program, moduleName));

    return wgslOutput;
}
