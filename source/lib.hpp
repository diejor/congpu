#pragma once

#include <string>

#include <webgpu/webgpu_cpp.h>

/**
 * @brief The core implementation of the executable.
 *
 * This class implements the main WebGPU initialization logic, abstracting the
 * process of creating a WebGPU instance, requesting an adapter and device, and
 * retrieving adapter info.
 */
struct Library
{
    /**
     * @brief Constructs the Library and initializes the name.
     */
    Library();

    /// The name of the project.
    std::string mName;

    /**
     * @brief Creates a WebGPU instance.
     * @return A valid wgpu::Instance, or nullptr if creation failed.
     */
    wgpu::Instance CreateInstance();

    /**
     * @brief Synchronously requests a WebGPU adapter.
     * @param instance The instance from which to request the adapter.
     * @return A valid wgpu::Adapter, or nullptr if the request failed.
     */
    wgpu::Adapter RequestAdapter(wgpu::Instance instance);

    /**
     * @brief Synchronously requests a WebGPU device from an adapter.
     * @param adapter The adapter from which to request the device.
     * @return A valid wgpu::Device, or nullptr if the request failed.
     */
    wgpu::Device RequestDevice(wgpu::Adapter adapter);

    /**
     * @brief Retrieves information about a given adapter.
     * @param adapter The adapter from which to retrieve information.
     * @return A filled wgpu::AdapterInfo structure.
     */
    wgpu::AdapterInfo GetAdapterInfo(wgpu::Adapter adapter);
};
