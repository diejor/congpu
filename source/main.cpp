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
#include "shaders/tools/gpu-printing.h"
#include "slang_compiler.hpp"

template<typename T, std::size_t B>
struct Align
{
    alignas(B) T value;
};

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


    Align<uint32_t, 4> parameters[4] = {2, 3, 2*3};
    LOG_WARN("Shape size: {}", sizeof(parameters));

    wgpu::BufferDescriptor buffer0Desc = {
        .label = "Size parameters",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopySrc
            | wgpu::BufferUsage::CopyDst,
        .size = sizeof(parameters),
        .mappedAtCreation = false,
    };
    wgpu::Buffer buffer0 = device.CreateBuffer(&buffer0Desc);
    queue.WriteBuffer(buffer0, 0, parameters, sizeof(parameters));

    Align<float, 4> data[12] = {2, 3, 4, 5, 6, 7, 8, 9};
    wgpu::BufferDescriptor buffer1Desc = {
        .label = "Buffer",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc
            | wgpu::BufferUsage::CopyDst,
        .size = sizeof(data),
        .mappedAtCreation = false,
    };

    wgpu::Buffer buffer1 = device.CreateBuffer(&buffer1Desc);
    queue.WriteBuffer(buffer1, 0, data, sizeof(data));

    size_t printBufferSize = 4 * 1024;

    wgpu::BufferDescriptor printBufferDesc = {
        .label = "Print Buffer",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc
            | wgpu::BufferUsage::CopyDst,
        .size = printBufferSize,
        .mappedAtCreation = false,
    };
    wgpu::Buffer printBuffer = device.CreateBuffer(&printBufferDesc);
    uint32_t zero = 0;
    queue.WriteBuffer(printBuffer, 0, &zero, sizeof(zero));

    wgpu::BufferDescriptor mapBufferDesc = {
        .label = "Map Buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
        .size = printBufferSize,
        .mappedAtCreation = false,
    };

    wgpu::Buffer mapBuffer = device.CreateBuffer(&mapBufferDesc);

    wgpu::BindGroupEntry bindGroupEntries[3] = {
        {
            .binding = 0,
            .buffer = buffer0,
            .offset = 0,
            .size = sizeof(parameters),
        },
        {
            .binding = 1,
            .buffer = buffer1,
            .offset = 0,
            .size = sizeof(data)
        },
        {
            .binding = 2,
            .buffer = printBuffer,
            .offset = 0,
            .size = printBufferSize,
        },
    };

    wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[3] = {
        {
            .binding = 0,
            .visibility = wgpu::ShaderStage::Compute,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::Uniform,
                    .hasDynamicOffset = false,
                    .minBindingSize = 0,
                },
        },
        {
            .binding = 1,
            .visibility = wgpu::ShaderStage::Compute,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::Storage,
                    .hasDynamicOffset = false,
                    .minBindingSize = 0,
                },
        },
        {
            .binding = 2,
            .visibility = wgpu::ShaderStage::Compute,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::Storage,
                    .hasDynamicOffset = false,
                    .minBindingSize = 0,
                },
        },
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
    computePassEncoder.DispatchWorkgroups(2, 3, 1);
    computePassEncoder.End();

    commandEncoder.CopyBufferToBuffer(
        printBuffer, 0, mapBuffer, 0, printBufferSize);

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
