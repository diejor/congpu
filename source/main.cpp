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

    std::filesystem::path path("../../source/shaders/");

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

    return EXIT_SUCCESS;
}
