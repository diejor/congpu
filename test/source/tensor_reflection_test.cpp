#include "tensor_reflection.hpp"

#include <catch2/catch_test_macros.hpp>

#include "slang_compiler.hpp"

TEST_CASE("Reflect RWTensorBuffer bindings", "[reflection]")
{
    const char* shader = R"(
import tensor;
static const int M = 8;
static const int N = 8;
RWTensorBuffer<float, int, int> input;
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    Tensor<float,M,N> tensorA;
    var inputRef = input;
    if(ltid.x==0){
        for(int j=0;j<N;++j)
            for(int i=0;i<M;++i)
                tensorA[i,j] = inputRef[i,j];
        for(int j=0;j<N;++j)
            for(int i=0;i<M;++i)
                inputRef[i,j] = tensorA[i,j] + 1.0f;
    }
}
)";

    auto shadersDir = std::string(SHADERS_DIR);
    slang_compiler::Compiler compiler({shadersDir});
    auto prog =
        compiler.CompileFromSource(shader, "tensor-buffer", "computeMain", {shadersDir});
    auto infoOpt =
        tensor_reflection::reflectTensorBuffer(prog.program.get(), "input");
    REQUIRE(infoOpt.has_value());
    auto info = *infoOpt;
    CHECK(info.dataBinding == 1);
    CHECK(info.dataSpace == 0);
    CHECK(info.shapeBinding == 0);
    CHECK(info.shapeSpace == 0);
    CHECK(info.shapeOffset == 0);
    CHECK(info.shapeSize == 32);
}
