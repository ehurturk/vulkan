#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <optional>

#include "defines.hpp"
#include "renderer/backend/vulkan/vulkan_swapchain.hpp"

namespace Renderer::Vulkan {
class Buffer;

struct QueueFamilyIndices {
    std::optional<U32> graphicsFamily;
    std::optional<U32> presentFamily;

    bool is_complete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

enum class QueueType {
    GRAPHICS_QUEUE = 0x1,
    TRANSFER_QUEUE = 0x2,
    COMPUTE_QUEUE = 0x4,
    PRESENT_QUEUE
};

class VulkanContext;

class VulkanDevice {
public:
    VulkanDevice(VulkanContext& context);
    ~VulkanDevice();

    VulkanDevice(const VulkanDevice& other) = delete;
    VulkanDevice& operator=(const VulkanDevice& other) = delete;

    void initialize();
    void destroy();

    Buffer create_buffer() const;

    inline VkDevice device() const { return m_Device; }
    inline VmaAllocator allocator() const { return m_Allocator; }
    inline QueueFamilyIndices queue_family_indices() const { return m_QueueIndices; }
    inline SwapchainSupportDetails swapchain_support_details() const {
        return m_SwapchainSupportDetails;
    }

private:
    QueueFamilyIndices find_queue_families(VkPhysicalDevice pd) const;
    SwapchainSupportDetails query_swap_chain_support(VkPhysicalDevice device) const;
    bool check_physical_device_extension_support(VkPhysicalDevice pd) const;
    bool is_physical_device_suitable(VkPhysicalDevice pd) const;

    void pick_physical_device();
    void create_device();
    void create_memory_allocator();

    VulkanContext& m_Context;

    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;

    VmaAllocator m_Allocator;

    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_DeviceExtensions;

    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

    QueueFamilyIndices m_QueueIndices;
    SwapchainSupportDetails m_SwapchainSupportDetails;
};

} // namespace Renderer::Vulkan
