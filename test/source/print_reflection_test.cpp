#include "print_reflection.hpp"

#include <catch2/catch_test_macros.hpp>

#include "slang_compiler.hpp"

TEST_CASE("Reflect gPrintBuffer bindings", "[reflection]")
{
    const char* shader = R"(
#include "tools/printing.slang"
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    println();
}
)";

    slang_compiler::Compiler compiler({SHADERS_DIR});
    auto prog =
        compiler.CompileFromSource(shader, "print-buffer", "computeMain");
    auto infoOpt = print_reflection::ReflectPrintBuffer(prog.program.get());
    REQUIRE(infoOpt.has_value());
    auto info = *infoOpt;
    CHECK(info.binding == 0);
    CHECK(info.space == 0);
}
