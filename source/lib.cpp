#include <cstdlib>
#include <iostream>

#include "lib.hpp"

#include <tracy/Tracy.hpp>

#include "webgpu//webgpu_cpp_print.h"

/// Constructs the Library and initializes the project name.
Library::Library()
    : mName("congpu")
{
}

wgpu::Instance Library::CreateInstance()
{
    ZoneScoped;
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
    ZoneScoped;
    wgpu::RequestAdapterOptions adapterOptions = {};
    wgpu::Adapter adapter = nullptr;

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
    ZoneScoped;
    wgpu::DeviceDescriptor deviceDescriptor {};
    auto errorCallback =
        [](wgpu::Device const&, wgpu::ErrorType type, wgpu::StringView message)
    {
        std::cerr << "Device error: " << message.data << "\n";
        std::cerr << "Error type: " << type << "\n";
    };
    deviceDescriptor.SetUncapturedErrorCallback(errorCallback);

    auto deviceLostCallback = [](wgpu::Device const& _device,
                                 wgpu::DeviceLostReason reason,
                                 wgpu::StringView message)
    {
        (void)_device;    // Suppress unused variable warning.
        std::cerr << "Device lost: " << message.data << "\n";
        std::cerr << "Reason: " << reason << "\n";
    };
    deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::WaitAnyOnly,
                                           deviceLostCallback);
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
    ZoneScoped;
    wgpu::DawnAdapterPropertiesPowerPreference
        powerProps {};    // Optional extra settings.
    wgpu::AdapterInfo info {};
    info.nextInChain = &powerProps;

    adapter.GetInfo(&info);
    return info;
}
