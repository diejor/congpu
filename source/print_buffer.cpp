#include "print_buffer.hpp"

namespace print_buffer
{
PrintBuffer::PrintBuffer(const print_reflection::PrintBufferReflection& refl,
                         wgpu::ShaderStage visibility)
    : mReflection(refl)
    , mVisibility(visibility)
{
    mLayoutEntry = {
        .binding = mReflection.binding,
        .visibility = mVisibility,
        .buffer =
            {
                .type = wgpu::BufferBindingType::Storage,
                .hasDynamicOffset = false,
                .minBindingSize = 0,
            },
    };

    mEntry.binding = mReflection.binding;
}

void PrintBuffer::Initialize(wgpu::Device device,
                             size_t byteSize,
                             wgpu::BufferUsage extraUsage)
{
    if (mInitialized) {
        return;
    }

    wgpu::BufferDescriptor desc = {
        .label = "print_buffer",
        .usage = wgpu::BufferUsage::Storage | extraUsage,
        .size = byteSize,
        .mappedAtCreation = false,
    };
    mBuffer = device.CreateBuffer(&desc);
    mEntry.buffer = mBuffer;
    mEntry.offset = 0;
    mEntry.size = byteSize;

    mInitialized = true;
}

const wgpu::BindGroupLayoutEntry* PrintBuffer::GetBindGroupLayoutEntries() const
{
    return &mLayoutEntry;
}

const wgpu::BindGroupEntry* PrintBuffer::GetBindGroupEntries() const
{
    return &mEntry;
}

size_t PrintBuffer::GetEntryCount() const
{
    return 1;
}

wgpu::Buffer PrintBuffer::GetBuffer() const
{
    return mBuffer;
}

}    // namespace print_buffer
