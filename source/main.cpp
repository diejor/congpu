#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <dawn/webgpu_cpp_print.h>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include "lib.hpp"
#include "logging_macros.h"
#include "result.h"
#include "slang_compiler.hpp"

Result<std::string, Error> loadTextFile(const std::filesystem::path& path)
{
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        return Error {"Could not open input file '" + path.string() + "'"};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main(int /*argc*/, char** /*argv*/)
{
    ZoneScoped;
    LOG_TRACE("Starting Slang to WGSL compilation...");
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

    std::cout << "Device: " << device.Get() << "\n";

    // Get the default queue.
    wgpu::Queue queue = device.GetQueue();

    std::filesystem::path path(SHADERS_DIR);
    LOG_TRACE("Loading Slang source from file: %s", path.string().c_str());

    std::vector<std::string> entryPoints = {"computeMain"};
    std::vector<std::string> includeDirectories = {path.string()};

    // Compile the Slang source to WGSL.
    std::string wgslSource;
    auto compileResult = slang_compiler::compileSlangToWgsl(
        "hello_world", entryPoints, includeDirectories);
    if (isError(compileResult)) {
        LOG_ERROR("Error compiling Slang to WGSL: %s",
                  std::get<1>(compileResult).message.c_str());
        return EXIT_FAILURE;
    }
    wgslSource = std::get<0>(compileResult);

    std::cout << "WGSL Source:\n" << wgslSource << "\n";

    LOG_TRACE("Closing Slang to WGSL compilation...");

    std::vector<float> data1 = {1.0f, 2.0f, 3.0f, 4.0f};

    wgpu::BufferDescriptor bufferDesc1 = {
        .label = "Buffer 1",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = data1.size() * sizeof(float),
        .mappedAtCreation = false,
    };
    wgpu::Buffer buffer1 = device.CreateBuffer(&bufferDesc1);

    wgpu::BufferDescriptor resultBufferDesc = {
        .label = "Result Buffer",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = data1.size() * sizeof(float),
        .mappedAtCreation = false,
    };

    wgpu::Buffer resultBuffer = device.CreateBuffer(&resultBufferDesc);

    wgpu::BufferDescriptor bufferDesc = {
        .label = "Result Buffer",
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
        .size = data1.size() * sizeof(float),
        .mappedAtCreation = false,
    };

    wgpu::Buffer mapBuffer = device.CreateBuffer(&bufferDesc);

    wgpu::BindGroupEntry bindGroupEntries[2] = {
        {
            .binding = 0,
            .buffer = buffer1,
            .offset = 0,
            .size = data1.size() * sizeof(float),
        },
        {
            .binding = 1,
            .buffer = resultBuffer,
            .offset = 0,
            .size = data1.size() * sizeof(float),
        },
    };

    wgpu::BindGroupLayoutEntry bindGroupLayoutEntries[2] = {
        {
            .binding = 0,
            .visibility = wgpu::ShaderStage::Compute,
            .buffer =
                {
                    .type = wgpu::BufferBindingType::ReadOnlyStorage,
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
    };

    wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc = {
        .label = "Bind Group Layout",
        .entryCount = 2,
        .entries = bindGroupLayoutEntries,
    };

    wgpu::BindGroupLayout bindGroupLayout =
        device.CreateBindGroupLayout(&bindGroupLayoutDesc);

    wgpu::BindGroupDescriptor bindGroupDesc = {
        .label = "Bind Group",
        .layout = bindGroupLayout,
        .entryCount = 2,
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

    queue.WriteBuffer(buffer1, 0, data1.data(), data1.size() * sizeof(float));

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
    computePassEncoder.DispatchWorkgroups(5, 1, 1);
    computePassEncoder.End();

    commandEncoder.CopyBufferToBuffer(
        resultBuffer, 0, mapBuffer, 0, data1.size() * sizeof(float));

    wgpu::CommandBufferDescriptor commandBufferDesc = {
        .label = "Command Buffer",
    };
    wgpu::CommandBuffer commandBuffer =
        commandEncoder.Finish(&commandBufferDesc);

    queue.Submit(1, &commandBuffer);
    LOG_TRACE("Command Buffer submitted.");

    std::vector<float> result(data1.size());

    auto mapCallback = [&mapBuffer, &result](wgpu::MapAsyncStatus status,
                                             wgpu::StringView message)
    {
        if (status == wgpu::MapAsyncStatus::Success) {
            LOG_TRACE("Buffer mapping succeeded: %s", message.data);
            const float* mappedData =
                static_cast<const float*>(mapBuffer.GetConstMappedRange(
                    0, result.size() * sizeof(float)));
            LOG_TRACE("Mapped data: %p",
                      reinterpret_cast<const void*>(mappedData));
            for (size_t i = 0; i < result.size(); ++i) {
                result[i] = mappedData[i];
                LOG_TRACE("Result[%zu]: %f", i, static_cast<double>(result[i]));
            }
            mapBuffer.Unmap();

        } else {
            LOG_ERROR("Buffer mapping failed: %s", message.data);
        }
    };
    wgpu::Future handle = mapBuffer.MapAsync(wgpu::MapMode::Read,
                                             0,
                                             data1.size() * sizeof(float),
                                             wgpu::CallbackMode::WaitAnyOnly,
                                             mapCallback);

    instance.WaitAny(handle, UINT64_MAX);
    LOG_TRACE("Buffer mapping completed.");

    return EXIT_SUCCESS;
}
