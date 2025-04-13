#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <webgpu/webgpu_cpp.h>
#include <webgpu/webgpu_cpp_print.h>

#include "lib.hpp"
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

    // Get and display adapter information.
    wgpu::AdapterInfo info = lib.GetAdapterInfo(adapter);
    std::cout << "VendorID: " << std::hex << info.vendorID << std::dec << "\n";
    std::cout << "Vendor: " << info.vendor << "\n";
    std::cout << "Architecture: " << info.architecture << "\n";
    std::cout << "DeviceID: " << std::hex << info.deviceID << std::dec << "\n";
    std::cout << "Name: " << info.device << "\n";
    std::cout << "Driver description: " << info.description << "\n";

    // Request the device.
    wgpu::Device device = lib.RequestDevice(adapter);
    if (device == nullptr) {
        return EXIT_FAILURE;
    }

    std::cout << "Device: " << device.Get() << "\n";

    // Get the default queue.
    wgpu::Queue queue = device.GetQueue();

    std::cout << std::filesystem::current_path() << "\n";
    std::filesystem::path path("../../source/shaders/hello_world.slang");
    std::cout << "Looking for shader file at: "
              << std::filesystem::absolute(path) << "\n";
    if (!std::filesystem::exists(path)) {
        std::cerr << "Shader file does not exist at: "
                  << std::filesystem::absolute(path) << "\n";
        return EXIT_FAILURE;
    }
    std::string slangSource;
    auto result = loadTextFile(path);
    if (isError(result)) {
        std::cerr << "Error loading file: " << std::get<1>(result).message
                  << "\n";
        return EXIT_FAILURE;
    }
    slangSource = std::get<0>(result);
    std::cout << "Loaded Slang source:\n" << slangSource << "\n";

    std::vector<std::string> entryPoints = {"computeMain"};
    std::vector<std::string> includeDirectories = {};

    // Compile the Slang source to WGSL.
    std::string wgslSource;
    auto compileResult = slang_compiler::compileSlangToWgsl(
        "hello_world", entryPoints, includeDirectories);
    if (isError(compileResult)) {
        std::cerr << "Error compiling Slang to WGSL: "
                  << std::get<1>(compileResult).message << "\n";
        return EXIT_FAILURE;
    }
    wgslSource = std::get<0>(compileResult);

    // Print the WGSL source code.
    std::cout << "WGSL Source:\n" << wgslSource << "\n";

    return EXIT_SUCCESS;
}
