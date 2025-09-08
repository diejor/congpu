#pragma once

#include <webgpu/webgpu_cpp.h>

#include "print_reflection.hpp"

namespace print_buffer
{
class PrintBuffer
{
  public:
    explicit PrintBuffer(
        const print_reflection::PrintBufferReflection& refl,
        wgpu::ShaderStage visibility = wgpu::ShaderStage::Compute);

    void Initialize(wgpu::Device device,
                    size_t byteSize,
                    wgpu::BufferUsage extraUsage = wgpu::BufferUsage::CopyDst
                        | wgpu::BufferUsage::CopySrc);

    [[nodiscard]] const wgpu::BindGroupLayoutEntry* GetBindGroupLayoutEntries()
        const;
    [[nodiscard]] const wgpu::BindGroupEntry* GetBindGroupEntries() const;
    [[nodiscard]] size_t GetEntryCount() const;

    [[nodiscard]] wgpu::Buffer GetBuffer() const;

  private:
    print_reflection::PrintBufferReflection mReflection;
    wgpu::ShaderStage mVisibility;

    wgpu::Buffer mBuffer {nullptr};

    wgpu::BindGroupLayoutEntry mLayoutEntry {};
    wgpu::BindGroupEntry mEntry {};
    bool mInitialized = false;
};
}    // namespace print_buffer
