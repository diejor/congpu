#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <dawn/webgpu_cpp_print.h>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include "lib.hpp"
#include "logging_macros.h"
#include "result.h"
#include "slang_compiler.hpp"

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
    LOG_TRACE("Loading Slang source from file: {}", path.string());

    std::vector<std::string> entryPoints = {"computeMain"};
    std::vector<std::string> includeDirectories = {path.string()};

    std::string wgslSource;
    auto compileResult = slang_compiler::compileSlangToWgsl(
        "matmul", entryPoints, includeDirectories);
    if (isError(compileResult)) {
        LOG_ERROR("Error compiling Slang to WGSL: {}",
                  std::get<1>(compileResult).message);
        return EXIT_FAILURE;
    }
    wgslSource = std::get<0>(compileResult);

    LOG_TRACE("WGSL source:\n{}", wgslSource);
}
