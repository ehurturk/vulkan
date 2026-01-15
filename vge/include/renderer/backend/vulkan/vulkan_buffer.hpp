#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "defines.hpp"

namespace Renderer::Vulkan {
class VulkanDevice;
class Buffer {
public:
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&&) noexcept;

    enum class Usage : U8 {
        TransferSource = 1 << 0,
        TransferDestination = 1 << 1,
        UniformBuffer = 1 << 4, // CPU updates
        StorageBuffer = 1 << 5, // GPU r/w
        IndexBuffer = 1 << 6,
        VertexBuffer = 1 << 7,
    };

    void upload(const void* data, size_t size, size_t offset = 0);

    void* mmap();
    void munmap();

    VkBuffer data() const { return m_Buffer; }
    VkDeviceSize size() const { return m_Size; }
    void* mapped_data() const { return m_MappedData; }

private:
    friend class VulkanDevice;

    Buffer(VulkanDevice& device, VkDeviceSize size, Usage usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

    Buffer(VulkanDevice& device, VkDeviceSize size, VkBufferUsageFlags usage,
        VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO);

    void destroy();

    VmaAllocation m_Allocation { VK_NULL_HANDLE };
    VmaAllocationInfo m_AllocationInfo {};
    VkBuffer m_Buffer { VK_NULL_HANDLE };
    void* m_MappedData { nullptr };

    VkDeviceSize m_Size {};

    VulkanDevice& r_Device;
};

} // namespace Renderer::Vulkan
