#pragma once
#include <string>
#include <vector>

#include <slang-com-ptr.h>
#include <slang.h>

namespace slang_compiler
{

struct SlangProgram
{
    Slang::ComPtr<slang::ISession> session;
    Slang::ComPtr<slang::IModule> module;
    Slang::ComPtr<slang::IComponentType> program;

    [[nodiscard]]
    std::string compileToWGSL() const;
};

class Compiler
{
  public:
    explicit Compiler(std::vector<std::string> const& baseIncludeDirs = {});

    Compiler(Compiler const&) = delete;
    Compiler& operator=(Compiler const&) = delete;
    Compiler(Compiler&&) = default;
    Compiler& operator=(Compiler&&) = default;

    [[nodiscard]]
    SlangProgram createProgram(
        std::string const& moduleName,
        std::string const& entryPoint,
        std::vector<std::string> const& extraIncludeDirs = {}) const;

  private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession;
    std::vector<std::string> m_baseIncludeDirs;
};

}    // namespace slang_compiler
