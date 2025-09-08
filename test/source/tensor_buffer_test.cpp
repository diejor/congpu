#include <vector>

#include "tensor_buffer.hpp"

#include <catch2/catch_test_macros.hpp>

#include "lib.hpp"
#include "slang_compiler.hpp"
#include "std140.hpp"
#include "tensor_reflection.hpp"

TEST_CASE("Create TensorBuffer bindings", "[tensor_buffer]")
{
    Library lib;
    wgpu::Instance instance = lib.CreateInstance();
    wgpu::Adapter adapter = lib.RequestAdapter(instance);
    wgpu::Device device = lib.RequestDevice(adapter);
    wgpu::Queue queue = device.GetQueue();

    constexpr int M = 4;
    constexpr int N = 4;

    const char* shader = R"(
import tensor;
static const int M = 4;
static const int N = 4;
RWTensorBuffer<float, int, int> input;
[numthreads(1,1,1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint3 ltid : SV_GroupThreadID)
{
    var r = input;
    if(ltid.x==0){
        for(int j=0;j<N;++j)
            for(int i=0;i<M;++i)
                r[i,j] = r[i,j];
    }
}
)";

    slang_compiler::Compiler compiler({SHADERS_DIR});
    auto prog = compiler.CompileFromSource(
        shader, "tensor-buffer", "computeMain");

    auto infoOpt =
        tensor_reflection::ReflectTensorBuffer(prog.program.get(), "input");
    REQUIRE(infoOpt.has_value());
    auto info = *infoOpt;

    tensor_buffer::TensorBuffer tb(info);
    tb.Initialize(device, M * N * sizeof(float));

    std140::Encoder encoder;
    {
        auto shape = encoder.beginStruct();
        {
            auto tuple = encoder.beginStruct();
            encoder.write(M);
            encoder.write(N);
        }
        encoder.write(M * N);
    }
    const auto& shape = encoder.data();
    REQUIRE(shape.size() == tb.GetShapeSize());
    queue.WriteBuffer(
        tb.GetShapeBuffer(), tb.GetShapeOffset(), shape.data(), shape.size());

    const wgpu::BindGroupLayoutEntry* layoutEntries =
        tb.GetBindGroupLayoutEntries();
    const wgpu::BindGroupEntry* entries = tb.GetBindGroupEntries();

    CHECK(layoutEntries[0].binding == info.shapeBinding);
    CHECK(layoutEntries[1].binding == info.dataBinding);
    CHECK(entries[1].size == M * N * sizeof(float));

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {
        .label = "TensorBuffer Layout",
        .entryCount = tb.GetEntryCount(),
        .entries = layoutEntries,
    };
    wgpu::BindGroupLayout bindGroupLayout =
        device.CreateBindGroupLayout(&bindGroupLayoutDesc);
    REQUIRE(bindGroupLayout != nullptr);

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .label = "TensorBuffer BindGroup",
        .layout = bindGroupLayout,
        .entryCount = tb.GetEntryCount(),
        .entries = entries,
    };
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bindGroupDesc);
    REQUIRE(bindGroup != nullptr);
}
