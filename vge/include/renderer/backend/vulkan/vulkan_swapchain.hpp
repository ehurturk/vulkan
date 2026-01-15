#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

namespace Platform {
class Window;
}

namespace Renderer::Vulkan {

class VulkanContext;

class VulkanSwapchain {
public:
    VulkanSwapchain(VulkanContext& context, Platform::Window& window);
    ~VulkanSwapchain();

private:
    Platform::Window& m_Window;

    VkSwapchainKHR m_Swapchain;

    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;

    std::vector<VkImage> m_SwapchainImages;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    void create_swapchain();
    void create_swapchain_image_views();
};

} // namespace Renderer::Vulkan
