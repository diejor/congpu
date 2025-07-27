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
    instanceDescriptor.requiredFeatureCount = 1;
    wgpu::InstanceFeatureName requiredFeatures[1] = {
        wgpu::InstanceFeatureName::TimedWaitAny};
    instanceDescriptor.requiredFeatures = requiredFeatures;
    wgpu::Instance instance = wgpu::CreateInstance(&instanceDescriptor);
    if (instance == nullptr) {
        LOG_ERROR("Failed to create an instance!");
        return nullptr;
    }
    return instance;
}

wgpu::Adapter Library::RequestAdapter(wgpu::Instance instance)
{
    ZoneScoped;

    wgpu::CallbackMode callbackMode = wgpu::CallbackMode::WaitAnyOnly;

    auto adapterCallback = [](wgpu::RequestAdapterStatus status,
                              wgpu::Adapter _adapter,
                              wgpu::StringView,
                              void* userdata)
    {
        if (status == wgpu::RequestAdapterStatus::Success) {
            *reinterpret_cast<wgpu::Adapter*>(userdata) = std::move(_adapter);
        }
    };

    constexpr wgpu::FeatureLevel kLevels[2] = {
        wgpu::FeatureLevel::Core, wgpu::FeatureLevel::Compatibility};

    wgpu::Adapter adapter = nullptr;
    wgpu::RequestAdapterOptions adapterOptions = {};

    for (wgpu::FeatureLevel level : kLevels) {
        adapterOptions.featureLevel = level;
        adapter = nullptr;    // reset before each attempt

        instance.WaitAny(
            instance.RequestAdapter(&adapterOptions,
                                    callbackMode,
                                    adapterCallback,
                                    reinterpret_cast<void*>(&adapter)),
            UINT64_MAX);

        if (adapter) {
            if (level == wgpu::FeatureLevel::Core) {
                LOG_INFO("Selected adapter with Core feature level");
            } else {
                LOG_WARN("Selected adapter with Compatibility feature level");
            }
            return adapter;
        }
    }

    LOG_ERROR("RequestAdapter failed! No adapter supports Core or Compatibility "
            "limits on this device.");
    return nullptr;
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
