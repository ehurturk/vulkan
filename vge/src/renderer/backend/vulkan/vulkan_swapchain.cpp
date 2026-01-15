#include "renderer/backend/vulkan/vulkan_swapchain.hpp"
#include "renderer/backend/vulkan/vulkan_context.hpp"
#include "platform/window/window.hpp"

namespace Renderer::Vulkan {

VulkanSwapchain::VulkanSwapchain(VulkanContext& context, Platform::Window& window)
    : m_Window(window) {
    create_swapchain();
    create_swapchain_image_views();
}

void VulkanSwapchain::create_swapchain() { }

void VulkanSwapchain::create_swapchain_image_views() { }

} // namespace Renderer::Vulkan
