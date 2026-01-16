#include "renderer/backend/vulkan/vulkan_swapchain.hpp"
#include "renderer/backend/vulkan/vulkan_context.hpp"
#include "renderer/backend/vulkan/vulkan_device.hpp"
#include "renderer/backend/vulkan/vulkan_utils.hpp"

#include "platform/window/window.hpp"

#include "core/logger.hpp"
#include <vulkan/vulkan_core.h>

namespace Renderer::Vulkan {

static VkSurfaceFormatKHR choose_swap_surface_format(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

// Choose a VkPresentModeKHR to base a swapchain's present mode to.
// Settle for a VK_PRESENT_MODE_KHR (triple-buffering without hard vsync).
// If no available present modes support VK_PRESENT_MODE_MAILBOX_KHR, settle
// for a VK_PRESENT_MODE_FIFO_KHR (strong vsync present mode).
static VkPresentModeKHR choose_swap_present_mode(
    const std::vector<VkPresentModeKHR>& availablePresentModes,
    const VkPresentModeKHR preferred_present_mode = VK_PRESENT_MODE_MAILBOX_KHR) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == preferred_present_mode)
            return availablePresentMode;
    }

    CORE_LOG_WARN("No available present modes support VK_PRESENT_MODE_MAILBOX_KHR, falling back to "
                  "{}!",
        Utils::to_string(availablePresentModes[0]));

    return availablePresentModes[0];
}

VulkanSwapchain::VulkanSwapchain(
    const VulkanDevice& device, const VkSurfaceKHR& surface, const Platform::Window& window)
    : m_Window(window)
    , r_Device(device)
    , r_Surface(surface) {
    create_swapchain();
    create_swapchain_image_views();
}

void VulkanSwapchain::initialize() {
    create_swapchain();
    create_swapchain_image_views();
}

void VulkanSwapchain::destroy() {
    // FIXME: DESTROY DEPTH IMAGE?
    // FIXME: DESTROY FRAMEBUFFER?

    for (auto imageView : m_SwapchainImageViews) {
        vkDestroyImageView(r_Device.device(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(r_Device.device(), m_Swapchain, nullptr);
}

void VulkanSwapchain::create_swapchain() {
    // NOTE: device must be created before the swapchain (as it should be) for this to return a
    // valid state
    SwapchainSupportDetails swapchainSupportDeatils = r_Device.swapchain_support_details();

    VkSurfaceFormatKHR surfaceFormat = choose_swap_surface_format(swapchainSupportDeatils.formats);
    VkPresentModeKHR presentMode = choose_swap_present_mode(swapchainSupportDeatils.presentModes);
    VkExtent2D extent = choose_swap_extent(swapchainSupportDeatils.capabilities);

    U32 imageCount = swapchainSupportDeatils.capabilities.minImageCount + 1;

    if (swapchainSupportDeatils.capabilities.maxImageCount > 0
        && imageCount > swapchainSupportDeatils.capabilities.maxImageCount) {
        imageCount = swapchainSupportDeatils.capabilities.maxImageCount;
    }

    m_SwapchainImageCount = imageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = r_Surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage
        = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // use swapchain images to render things

    auto [graphicsFamily, presentFamily] = r_Device.queue_family_indices();
    std::array<U32, 2> queueFamilyIndices
        = { graphicsFamily.value_or(0), presentFamily.value_or(0) };

    if (graphicsFamily != presentFamily) {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
    } else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }

    swapchainCreateInfo.preTransform = swapchainSupportDeatils.capabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VULKAN_CHECK(
        vkCreateSwapchainKHR(r_Device.device(), &swapchainCreateInfo, nullptr, &m_Swapchain));

    vkGetSwapchainImagesKHR(r_Device.device(), m_Swapchain, &imageCount, nullptr);
    m_SwapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(r_Device.device(), m_Swapchain, &imageCount, m_SwapchainImages.data());

    CORE_LOG_INFO("[VulkanSwapchain]: Swapchain created with {} images", imageCount);

    m_SwapchainImageFormat = surfaceFormat.format;
    m_SwapchainExtent = extent;
}

void VulkanSwapchain::create_swapchain_image_views() {
    m_SwapchainImageViews.resize(m_SwapchainImages.size());
    for (size_t i = 0; i < m_SwapchainImages.size(); i++) {
        // TODO: Maybe use the image view abstraction in GpuImage???

        VkImageViewCreateInfo viewInfo {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_SwapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_SwapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        VULKAN_CHECK(vkCreateImageView(r_Device.device(), &viewInfo, nullptr, &imageView));

        m_SwapchainImageViews[i] = imageView;
    }
}

VkExtent2D VulkanSwapchain::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<U32>::max()) {
        return capabilities.currentExtent;
    }

    const Platform::Window::Extent extent = m_Window.getExtentPixel();

    VkExtent2D ext = { extent.width, extent.height };
    ext.width = std::clamp(
        ext.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    ext.height = std::clamp(
        ext.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return ext;
}

} // namespace Renderer::Vulkan
