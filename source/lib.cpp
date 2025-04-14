#include <cstdlib>
#include <iostream>

#include "lib.hpp"

/// Constructs the Library and initializes the project name.
Library::Library()
    : mName("congpu")
{
}

wgpu::Instance Library::CreateInstance()
{
    wgpu::InstanceDescriptor instanceDescriptor {};
    instanceDescriptor.capabilities.timedWaitAnyEnable = true;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    if (instance == nullptr) {
        std::cerr << "Instance creation failed!\n";
    }
    return instance;
}

wgpu::Adapter Library::RequestAdapter(wgpu::Instance instance)
{
    wgpu::RequestAdapterOptions adapterOptions = {};
    wgpu::Adapter adapter = nullptr;

    // Callback to capture the adapter from the asynchronous request.
    auto adapterCallback = [](wgpu::RequestAdapterStatus status,
                              wgpu::Adapter _adapter,
                              const char* message,
                              void* userdata)
    {
        if (status != wgpu::RequestAdapterStatus::Success) {
            std::cerr << "Failed to get an adapter: " << message << "\n";
            return;
        }
        *static_cast<wgpu::Adapter*>(userdata) = std::move(_adapter);
    };

    auto callbackMode = wgpu::CallbackMode::WaitAnyOnly;
    void* userdata = &adapter;

    // Initiate the adapter request and wait synchronously.
    instance.WaitAny(
        instance.RequestAdapter(
            &adapterOptions, callbackMode, adapterCallback, userdata),
        UINT64_MAX);

    if (adapter == nullptr) {
        std::cerr << "RequestAdapter failed!\n";
    }
    return adapter;
}

wgpu::Device Library::RequestDevice(wgpu::Adapter adapter)
{
    wgpu::DeviceDescriptor deviceDescriptor {};
    wgpu::Device device = nullptr;

    // Callback to capture the device from the asynchronous request.
    auto deviceCallback = [](wgpu::RequestDeviceStatus status,
                             wgpu::Device _device,
                             const char* message,
                             void* userdata)
    {
        if (status != wgpu::RequestDeviceStatus::Success) {
            std::cerr << "Failed to get a device: " << message << "\n";
            return;
        }
        *static_cast<wgpu::Device*>(userdata) = std::move(_device);
    };

    auto deviceCallbackMode = wgpu::CallbackMode::WaitAnyOnly;
    void* deviceUserdata = &device;

    // Initiate the device request and wait synchronously.
    adapter.GetInstance().WaitAny(adapter.RequestDevice(&deviceDescriptor,
                                                        deviceCallbackMode,
                                                        deviceCallback,
                                                        deviceUserdata),
                                  UINT64_MAX);

    if (device == nullptr) {
        std::cerr << "RequestDevice failed!\n";
    }
    return device;
}

wgpu::AdapterInfo Library::GetAdapterInfo(wgpu::Adapter adapter)
{
    wgpu::DawnAdapterPropertiesPowerPreference
        powerProps {};    // Optional extra settings.
    wgpu::AdapterInfo info {};
    info.nextInChain = &powerProps;

    adapter.GetInfo(&info);
    return info;
}
