#include "print_buffer.hpp"

#include <catch2/catch_test_macros.hpp>

#include "lib.hpp"
#include "print_reflection.hpp"
#include "slang_compiler.hpp"

TEST_CASE("Create PrintBuffer bindings", "[print_buffer]")
{
    Library lib;
    wgpu::Instance instance = lib.CreateInstance();
    wgpu::Adapter adapter = lib.RequestAdapter(instance);
    wgpu::Device device = lib.RequestDevice(adapter);

    constexpr size_t BufferWords = 128;
    constexpr size_t BufferSize = BufferWords * sizeof(uint32_t);

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

    print_buffer::PrintBuffer pb(info);
    pb.Initialize(device, BufferSize);

    const wgpu::BindGroupLayoutEntry* layoutEntry =
        pb.GetBindGroupLayoutEntries();
    const wgpu::BindGroupEntry* entry = pb.GetBindGroupEntries();

    CHECK(layoutEntry[0].binding == info.binding);
    CHECK(entry[0].size == BufferSize);

    wgpu::BindGroupLayoutDescriptor layoutDesc = {
        .label = "PrintBuffer Layout",
        .entryCount = pb.GetEntryCount(),
        .entries = layoutEntry,
    };
    wgpu::BindGroupLayout bindGroupLayout =
        device.CreateBindGroupLayout(&layoutDesc);
    REQUIRE(bindGroupLayout != nullptr);

    wgpu::BindGroupDescriptor groupDesc = {
        .label = "PrintBuffer BindGroup",
        .layout = bindGroupLayout,
        .entryCount = pb.GetEntryCount(),
        .entries = entry,
    };
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&groupDesc);
    REQUIRE(bindGroup != nullptr);
}
