#include <cstdlib>
#include <filesystem>
#include <iostream>

#include <dawn/webgpu_cpp_print.h>
#include <tracy/Tracy.hpp>
#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include "lib.hpp"
#include "logging_macros.h"
#include "shaders/gpu-printing.h"
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
    slang_compiler::Compiler compiler({path.string()});

    slang_compiler::SlangProgram program =
        compiler.createProgram("matmul", "computeMain");

    std::string wgslSource = program.compileToWGSL();

    LOG_TRACE("WGSL source:\n{}", wgslSource);
}
