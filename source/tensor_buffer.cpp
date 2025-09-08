#include "tensor_buffer.hpp"

namespace tensor_buffer
{
TensorBuffer::TensorBuffer(
    const tensor_reflection::TensorBufferReflection& refl,
    wgpu::ShaderStage visibility)
    : mReflection(refl)
    , mVisibility(visibility)
{
    mLayoutEntries[0] = {
        .binding = mReflection.shapeBinding,
        .visibility = mVisibility,
        .buffer =
            {
                .type = wgpu::BufferBindingType::Uniform,
                .hasDynamicOffset = false,
                .minBindingSize = mReflection.shapeSize,
            },
    };
    mLayoutEntries[1] = {
        .binding = mReflection.dataBinding,
        .visibility = mVisibility,
        .buffer =
            {
                .type = wgpu::BufferBindingType::Storage,
                .hasDynamicOffset = false,
                .minBindingSize = 0,
            },
    };

    mEntries[0].binding = mReflection.shapeBinding;
    mEntries[1].binding = mReflection.dataBinding;
}

void TensorBuffer::Initialize(wgpu::Device device,
                              size_t byteSize,
                              wgpu::BufferUsage extraUsage)
{
    if (mInitialized) {
        return;
    }

    wgpu::BufferDescriptor shapeDesc = {
        .label = "tensor_shape",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = mReflection.shapeOffset + mReflection.shapeSize,
        .mappedAtCreation = false,
    };
    mShapeBuffer = device.CreateBuffer(&shapeDesc);
    mEntries[0].buffer = mShapeBuffer;
    mEntries[0].offset = 0;
    mEntries[0].size = shapeDesc.size;

    wgpu::BufferDescriptor dataDesc = {
        .label = "tensor_data",
        .usage = wgpu::BufferUsage::Storage | extraUsage,
        .size = byteSize,
        .mappedAtCreation = false,
    };
    mDataBuffer = device.CreateBuffer(&dataDesc);
    mEntries[1].buffer = mDataBuffer;
    mEntries[1].offset = 0;
    mEntries[1].size = byteSize;

    mInitialized = true;
}

const wgpu::BindGroupLayoutEntry* TensorBuffer::GetBindGroupLayoutEntries()
    const
{
    return mLayoutEntries;
}

const wgpu::BindGroupEntry* TensorBuffer::GetBindGroupEntries() const
{
    return mEntries;
}

size_t TensorBuffer::GetEntryCount() const
{
    return 2;
}

wgpu::Buffer TensorBuffer::GetDataBuffer() const
{
    return mDataBuffer;
}

wgpu::Buffer TensorBuffer::GetShapeBuffer() const
{
    return mShapeBuffer;
}

size_t TensorBuffer::GetShapeOffset() const
{
    return mReflection.shapeOffset;
}

size_t TensorBuffer::GetShapeSize() const
{
    return mReflection.shapeSize;
}

}    // namespace tensor_buffer
