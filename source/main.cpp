#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <span>
#include <vector>

#include <dawn/webgpu_cpp_print.h>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include "lib.hpp"
#include "logging_macros.h"
#include "print_buffer.hpp"
#include "print_reflection.hpp"
#include "shaders/tools/gpu-printing.h"
#include "slang_compiler.hpp"
#include "std140.hpp"
#include "tensor_buffer.hpp"
#include "tensor_reflection.hpp"

int main(int /*argc*/, char** /*argv*/)
{
    ZoneScoped;
    Library lib;

    // Create the instance.
    wgpu::Instance instance = lib.CreateInstance();
    if (instance == nullptr) {
        return EXIT_FAILURE;
    }

    // Request the adapter.
    wgpu::Adapter adapter = lib.RequestAdapter(instance);
    if (adapter == nullptr) {
        return EXIT_FAILURE;
    }

    // Request the device.
    wgpu::Device device = lib.RequestDevice(adapter);
    if (device == nullptr) {
        return EXIT_FAILURE;
    }

    // Get the default queue.
    wgpu::Queue queue = device.GetQueue();

    std::filesystem::path path(SHADERS_DIR);
    slang_compiler::Compiler compiler({path.string()});

    slang_compiler::SlangProgram slangProgram =
        compiler.CreateProgram("matmul", "computeMain");

    std::string wgslSource = slangProgram.compileToWGSL();

    LOG_TRACE("WGSL source:\n{}", wgslSource);

    GPUPrinting gpuPrinting;
    gpuPrinting.loadStrings(slangProgram.program->getLayout());

    float data[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

    auto tensorInfoOpt = tensor_reflection::ReflectTensorBuffer(
        slangProgram.program.get(), "input");
    if (!tensorInfoOpt) {
        return EXIT_FAILURE;
    }
    tensor_buffer::TensorBuffer tensor(*tensorInfoOpt);
    tensor.Initialize(device, sizeof(data));

    auto printInfoOpt =
        print_reflection::ReflectPrintBuffer(slangProgram.program.get());
    if (!printInfoOpt) {
        return EXIT_FAILURE;
    }
    print_buffer::PrintBuffer printBuf(*printInfoOpt);
    size_t printBufferSize = 4 * 1024;
    printBuf.Initialize(device, printBufferSize);
    uint32_t zero = 0;
    queue.WriteBuffer(printBuf.GetBuffer(), 0, &zero, sizeof(zero));

    std140::Encoder encoder;
    {
        auto shape = encoder.beginStruct();
        {
            auto tuple = encoder.beginStruct();
            encoder.write<int32_t>(3);
            encoder.write<int32_t>(4);
        }
        encoder.write<int32_t>(12);
    }
    const std::vector<std::byte>& shape = encoder.data();
    queue.WriteBuffer(tensor.GetShapeBuffer(),
                      tensor.GetShapeOffset(),
                      shape.data(),
                      shape.size());
    queue.WriteBuffer(tensor.GetDataBuffer(), 0, data, sizeof(data));

    wgpu::BufferDescriptor mapBufferDesc = {
        .label = "Map Buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
        .size = printBufferSize,
        .mappedAtCreation = false,
    };

    wgpu::Buffer mapBuffer = device.CreateBuffer(&mapBufferDesc);

    const wgpu::BindGroupLayoutEntry* tensorLayoutEntries =
        tensor.GetBindGroupLayoutEntries();
    const wgpu::BindGroupEntry* tensorEntries = tensor.GetBindGroupEntries();
    const wgpu::BindGroupLayoutEntry* printLayoutEntry =
        printBuf.GetBindGroupLayoutEntries();
    const wgpu::BindGroupEntry* printEntry = printBuf.GetBindGroupEntries();

    wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[3] = {
        tensorLayoutEntries[0],
        tensorLayoutEntries[1],
        *printLayoutEntry,
    };

    wgpu::BindGroupEntry bindGroupEntries[3] = {
        tensorEntries[0],
        tensorEntries[1],
        *printEntry,
    };

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {
        .label = "Bind Group Layout",
        .entryCount = 3,
        .entries = bindGroupLayoutEntries,
    };

    wgpu::BindGroupLayout bindGroupLayout =
        device.CreateBindGroupLayout(&bindGroupLayoutDesc);

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .label = "Bind Group",
        .layout = bindGroupLayout,
        .entryCount = 3,
        .entries = bindGroupEntries,
    };

    wgpu::BindGroup bindGroup = device.CreateBindGroup(&bindGroupDesc);

    wgpu::PipelineLayoutDescriptor pipelineLayoutDesc = {
        .label = "Pipeline Layout",
        .bindGroupLayoutCount = 1,
        .bindGroupLayouts = &bindGroupLayout,
    };

    wgpu::PipelineLayout pipelineLayout =
        device.CreatePipelineLayout(&pipelineLayoutDesc);

    wgpu::ShaderSourceWGSL shader = {};
    shader.code = wgslSource.c_str();

    wgpu::ShaderModuleDescriptor shaderModuleDesc = {
        .nextInChain = reinterpret_cast<wgpu::ChainedStruct*>(&shader),
        .label = "Shader Module",
    };

    wgpu::ShaderModule shaderModule =
        device.CreateShaderModule(&shaderModuleDesc);

    wgpu::ComputePipelineDescriptor computePipelineDesc = {
        .label = "Compute Pipeline",
        .layout = pipelineLayout,
        .compute =
            {
                .module = shaderModule,
                .entryPoint = "computeMain",
                .constantCount = 0,
                .constants = nullptr,
            },
    };

    wgpu::ComputePipeline computePipeline =
        device.CreateComputePipeline(&computePipelineDesc);

    wgpu::CommandEncoderDescriptor commandEncoderDesc = {
        .label = "Command Encoder",
    };

    wgpu::CommandEncoder commandEncoder =
        device.CreateCommandEncoder(&commandEncoderDesc);

    wgpu::ComputePassDescriptor computePassDesc = {
        .label = "Compute Pass",
    };
    wgpu::ComputePassEncoder computePassEncoder =
        commandEncoder.BeginComputePass(&computePassDesc);

    computePassEncoder.SetPipeline(computePipeline);
    computePassEncoder.SetBindGroup(0, bindGroup);
    computePassEncoder.DispatchWorkgroups(1, 4, 1);
    computePassEncoder.End();

    commandEncoder.CopyBufferToBuffer(
        printBuf.GetBuffer(), 0, mapBuffer, 0, printBufferSize);

    wgpu::CommandBufferDescriptor commandBufferDesc = {
        .label = "Command Buffer",
    };
    wgpu::CommandBuffer commandBuffer =
        commandEncoder.Finish(&commandBufferDesc);

    queue.Submit(1, &commandBuffer);

    auto mapCallback = [&mapBuffer, &gpuPrinting, &printBufferSize](
                           wgpu::MapAsyncStatus status, wgpu::StringView)
    {
        if (status == wgpu::MapAsyncStatus::Success) {
            const void* mappedData =
                mapBuffer.GetConstMappedRange(0, printBufferSize);

            gpuPrinting.processGPUPrintCommands(mappedData, printBufferSize);
            mapBuffer.Unmap();
        }
    };

    wgpu::Future handle = mapBuffer.MapAsync(wgpu::MapMode::Read,
                                             0,
                                             printBufferSize,
                                             wgpu::CallbackMode::WaitAnyOnly,
                                             mapCallback);

    instance.WaitAny(handle, UINT64_MAX);
}
