#pragma once

#include <webgpu/webgpu_cpp.h>

#include "tensor_reflection.hpp"

namespace tensor_buffer
{
class TensorBuffer
{
  public:
    TensorBuffer(const tensor_reflection::TensorBufferReflection& refl,
                 wgpu::ShaderStage visibility = wgpu::ShaderStage::Compute);

    void Initialize(wgpu::Device device,
                    size_t byteSize,
                    wgpu::BufferUsage extraUsage = wgpu::BufferUsage::CopyDst
                        | wgpu::BufferUsage::CopySrc);

    [[nodiscard]] const wgpu::BindGroupLayoutEntry* GetBindGroupLayoutEntries()
        const;
    [[nodiscard]] const wgpu::BindGroupEntry* GetBindGroupEntries() const;
    [[nodiscard]] size_t GetEntryCount() const;

    [[nodiscard]] wgpu::Buffer GetDataBuffer() const;
    [[nodiscard]] wgpu::Buffer GetShapeBuffer() const;
    [[nodiscard]] size_t GetShapeOffset() const;
    [[nodiscard]] size_t GetShapeSize() const;

  private:
    tensor_reflection::TensorBufferReflection mReflection;
    wgpu::ShaderStage mVisibility;

    wgpu::Buffer mDataBuffer {nullptr};
    wgpu::Buffer mShapeBuffer {nullptr};

    wgpu::BindGroupLayoutEntry mLayoutEntries[2] {};
    wgpu::BindGroupEntry mEntries[2] {};
    bool mInitialized = false;
};
}    // namespace tensor_buffer
