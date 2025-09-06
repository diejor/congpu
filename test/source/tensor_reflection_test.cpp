#include <filesystem>

#include <catch2/catch_test_macros.hpp>

#include "slang_compiler.hpp"
#include "tensor_reflection.hpp"

TEST_CASE("TensorBuffer reflection", "[reflection]")
{
    std::filesystem::path shadersPath(SHADERS_DIR);
    std::filesystem::path testShadersPath(TEST_SHADERS_DIR);
    slang_compiler::Compiler compiler({shadersPath.string(), testShadersPath.string()});

    slang_compiler::SlangProgram program =
        compiler.CreateProgram("tensor-buffer", "computeMain");

    auto infos = tensor_reflection::reflectTensorBuffers(program.program->getLayout());
    REQUIRE(infos.size() == 1);
    const auto& info = infos[0];
    REQUIRE(info.name == "input");
    REQUIRE(info.data.category != slang::ParameterCategory::Uniform);
    REQUIRE(info.shapeBuffer.category != slang::ParameterCategory::Uniform);
    REQUIRE(info.data.index != info.shapeBuffer.index);
}
