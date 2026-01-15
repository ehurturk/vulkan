#include "renderer/backend/vulkan/vulkan_buffer.hpp"
#include "renderer/backend/vulkan/vulkan_device.hpp"
#include "core/assert.hpp"
#include "core/logger.hpp"
#include "vk_mem_alloc.h"

namespace Renderer::Vulkan {

Buffer::Buffer(
    VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
    : m_Size { size }
    , r_Device { device } {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.flags = 0;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    // VMA:
    // Get a memory type that is located on the GPU VRAM and is
    // accessible by the host.
    // TODO: works for vertex/index buffers where cpu-gpu mapping is needed.
    //       what about for other types of buffers?
    VmaAllocationCreateInfo allocCI {};
    allocCI.priority = 1.0f;
    allocCI.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT
        | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocCI.usage = memoryUsage;

    // TODO: vulkan check this!
    vmaCreateBuffer(
        r_Device.allocator(), &bufferInfo, &allocCI, &m_Buffer, &m_Allocation, &m_AllocationInfo);

    m_MappedData = m_AllocationInfo.pMappedData;
}

Buffer::~Buffer() { destroy(); }

void Buffer::upload(const void* data, size_t size, size_t offset) {
    ASSERT_MSG(data != nullptr, "Uploaded data is null.");
    ASSERT_MSG(m_MappedData != nullptr, "Buffer is not host-visible.");
    ASSERT_MSG(offset + size <= m_Size, "Upload data exceeds buffer size.");
    memcpy(static_cast<char*>(m_MappedData) + offset, data, size);
}

void* Buffer::mmap() {
    if (!m_MappedData) {
        vmaMapMemory(r_Device.allocator(), m_Allocation, &m_MappedData);
    }

    return m_MappedData;
}

void Buffer::munmap() {
    if (m_MappedData) {
        vmaUnmapMemory(r_Device.allocator(), m_Allocation);
        m_MappedData = nullptr;
    }
}

void Buffer::destroy() {
    CORE_LOG_INFO("[Vulkan::Buffer]: Destroying a buffer...");
    if (m_Buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(r_Device.allocator(), m_Buffer, m_Allocation);
        m_Buffer = VK_NULL_HANDLE;
        m_Allocation = VK_NULL_HANDLE;
        m_MappedData = nullptr;
    }
}

} // namespace Renderer::Vulkan
