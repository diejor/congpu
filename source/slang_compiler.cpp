#include <algorithm>
#include <array>
#include <iostream>

#include "slang_compiler.hpp"

#include "logging_macros.h"

// ────────────────────────────────────────────────────────────
// Small helpers
// ────────────────────────────────────────────────────────────
namespace
{

void diagnoseIfNeeded(Slang::ComPtr<slang::IBlob> const& blob)
{
    if (!blob) {
        return;
    }
    std::cerr << "[slang] diagnostics:\n"
              << static_cast<const char*>(blob->getBufferPointer()) << '\n';
}

Slang::ComPtr<slang::ISession> createSession(
    slang::IGlobalSession* global, std::vector<std::string> const& searchPaths)
{
    slang::SessionDesc desc {};

    slang::TargetDesc target {};
    target.format = SLANG_WGSL;
    desc.targets = &target;
    desc.targetCount = 1;

    // convert search paths to const char*
    std::vector<char const*> cPaths;
    cPaths.reserve(searchPaths.size());
    for (auto& p : searchPaths) {
        cPaths.push_back(p.c_str());
    }

    desc.searchPaths = cPaths.data();
    desc.searchPathCount = static_cast<SlangInt>(cPaths.size());

    Slang::ComPtr<slang::ISession> session;
    global->createSession(desc, session.writeRef());
    return session;
}

}    // namespace

// ────────────────────────────────────────────────────────────
//               SlangProgram implementation
// ────────────────────────────────────────────────────────────
std::string slang_compiler::SlangProgram::compileToWGSL() const
{
    if (!program) {
        return {};
    }

    Slang::ComPtr<slang::IBlob> codeBlob, diagBlob;
    if (SLANG_FAILED(program->getTargetCode(
            0, codeBlob.writeRef(), diagBlob.writeRef())))
    {
        diagnoseIfNeeded(diagBlob);
        return {};
    }
    return std::string(static_cast<char const*>(codeBlob->getBufferPointer()),
                       codeBlob->getBufferSize());
}

// ────────────────────────────────────────────────────────────
//                 Compiler implementation
// ────────────────────────────────────────────────────────────
using namespace slang_compiler;
using Slang::ComPtr;

Compiler::Compiler(std::vector<std::string> const& baseIncludeDirs)
    : m_baseIncludeDirs(baseIncludeDirs)
{
    slang::createGlobalSession(m_globalSession.writeRef());
}

SlangProgram Compiler::createProgram(
    std::string const& moduleName,
    std::string const& entryPoint,
    std::vector<std::string> const& extraIncludeDirs) const
{
    // 1. ---- build search path list ------------------------------------------
    std::vector<std::string> searchPaths = m_baseIncludeDirs;
    searchPaths.insert(
        searchPaths.end(), extraIncludeDirs.begin(), extraIncludeDirs.end());

    // 2. ---- create a per‑compile ISession -----------------------------------
    ComPtr<slang::ISession> session =
        createSession(m_globalSession, searchPaths);
    if (!session) {
        LOG_ERROR("Failed to create Slang session");
        return {};
    }

    // 3. ---- load module
    // ------------------------------------------------------
    ComPtr<slang::IModule> module;
    {
        ComPtr<slang::IBlob> diagnosticsBlob;
        module =
            session->loadModule(moduleName.c_str(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module) {
            LOG_ERROR("Failed to load module: {}", moduleName);
            return {};
        }
        LOG_TRACE("Loaded module: {}", moduleName);
    }

    // 4. ---- find entry point + composite component --------------------------
    ComPtr<slang::IEntryPoint> entry;
    module->findEntryPointByName(entryPoint.c_str(), entry.writeRef());

    std::array<slang::IComponentType*, 2> parts {module.get(), entry.get()};

    ComPtr<slang::IComponentType> composite;
    {
        ComPtr<slang::IBlob> diagnosticsBlob;
        session->createCompositeComponentType(parts.data(),
                                              parts.size(),
                                              composite.writeRef(),
                                              diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!composite) {
            LOG_ERROR("Failed to create composite component");
            return {};
        }
        LOG_TRACE("Created composite component");
    }

    // 5. ---- link
    // -------------------------------------------------------------
    ComPtr<slang::IComponentType> linked;
    {
        ComPtr<slang::IBlob> diagnosticsBlob;
        composite->link(linked.writeRef(), diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!linked) {
            LOG_ERROR("Failed to link program");
            return {};
        }
        LOG_TRACE("Linked program");
    }

    // 6. ---- package everything into SlangProgram ----------------------------
    return SlangProgram {.session = std::move(session),
                         .module = std::move(module),
                         .program = std::move(linked)};
}
