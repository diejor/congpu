#include <cstdlib>
#include <iostream>

#include "lib.hpp"

#include <tracy/Tracy.hpp>

#include "logging_macros.h"

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
                              wgpu::StringView message,
                              void* userdata)
    {
        if (status != wgpu::RequestAdapterStatus::Success) {
            LOG_ERROR("Failed to get an adapter: {}", message);
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
        LOG_ERROR(
            "RequestAdapter failed! Please check that WebGPU is supported by "
            "your GPU.");
    }
    return adapter;
}

wgpu::Device Library::RequestDevice(wgpu::Adapter adapter)
{
    ZoneScoped;
    wgpu::DeviceDescriptor deviceDescriptor {};
    auto errorCallback =
        [](wgpu::Device const&, wgpu::ErrorType type, wgpu::StringView message)
    { LOG_ERROR("{}, {}", type, message); };
    deviceDescriptor.SetUncapturedErrorCallback(errorCallback);

    auto deviceLostCallback = [](wgpu::Device const&,
                                 wgpu::DeviceLostReason reason,
                                 wgpu::StringView message)
    { LOG_ERROR("{}, {}", reason, message); };
    deviceDescriptor.SetDeviceLostCallback(wgpu::CallbackMode::WaitAnyOnly,
                                           deviceLostCallback);
    wgpu::Device device = nullptr;

    auto deviceCallback = [](wgpu::RequestDeviceStatus status,
                             wgpu::Device _device,
                             const char* message,
                             void* userdata)
    {
        if (status != wgpu::RequestDeviceStatus::Success) {
            LOG_ERROR("Failed to get a device: {}", message);
            return;
        }
        *static_cast<wgpu::Device*>(userdata) = std::move(_device);
    };

    auto deviceCallbackMode = wgpu::CallbackMode::WaitAnyOnly;
    void* deviceUserdata = &device;

    adapter.GetInstance().WaitAny(adapter.RequestDevice(&deviceDescriptor,
                                                        deviceCallbackMode,
                                                        deviceCallback,
                                                        deviceUserdata),
                                  UINT64_MAX);

    if (device == nullptr) {
        LOG_ERROR("RequestDevice failed! Not sure why.");
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
