#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

#include "defines.hpp"

namespace Platform {
class Window;
}

namespace Renderer::Vulkan {

class VulkanDevice;
class VulkanContext;

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain {
public:
    // TODO: window stores surface?
    VulkanSwapchain(
        const VulkanDevice& device, const VkSurfaceKHR& surface, const Platform::Window& window);
    ~VulkanSwapchain();

    void initialize();
    void destroy();

    inline VkSwapchainKHR swapchain() const { return m_Swapchain; }
    inline VkFormat image_format() const { return m_SwapchainImageFormat; }
    inline U32 image_count() const { return m_SwapchainImageCount; }
    inline VkExtent2D extent() const { return m_SwapchainExtent; }

    inline const std::vector<VkImage>& images() const { return m_SwapchainImages; }
    inline const std::vector<VkImageView>& image_views() const { return m_SwapchainImageViews; }
    inline const std::vector<VkFramebuffer>& framebuffers() const {
        return m_SwapchainFramebuffers;
    }

private:
    const Platform::Window& m_Window;
    const VulkanDevice& r_Device;

    VkSwapchainKHR m_Swapchain;
    const VkSurfaceKHR& r_Surface; // i know these r handles but using a ref makes it easier for me
                                   // to reason about its borrowed.

    // TODO: Replace both these vectors into a single vector of:
    //       std::vector<GpuImage> m_SwapchainImages;
    //       later
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkImage> m_SwapchainImages;

    std::vector<VkFramebuffer> m_SwapchainFramebuffers;

    U32 m_SwapchainImageCount;

    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    void create_swapchain();
    void create_swapchain_image_views();

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
};

} // namespace Renderer::Vulkan
