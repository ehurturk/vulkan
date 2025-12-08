#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "renderer/backend/renderer.hpp"
#include "defines.hpp"

namespace Platform {
class Window;
}

namespace Renderer {

class VulkanRenderer final : public RendererBackend {
   public:
    VulkanRenderer(Platform::Window* window);
    ~VulkanRenderer() override;

    void initialize(const RendererConfig& cfg) override;
    void shutdown() override;
    void draw_frame() override;

   private:
    struct VkState {
        VkInstance instance = VK_NULL_HANDLE;
        VkInstanceCreateInfo createInfo;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        bool validation = false;
    };

    struct QueueFamilyIndices {
        std::optional<U32> graphicsFamily;
        std::optional<U32> presentFamily;

        bool is_complete() { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    std::vector<const char*> getRequiredExtensions();

    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_graphics_pipeline();
    void create_renderpass();
    void create_framebuffers();
    void create_commandpool();
    void create_commandbuffer();
    void create_sync_objects();

    bool is_physical_device_suitable(VkPhysicalDevice device);

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);

    SwapchainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool check_validation_layer_support();
    void destroy_debug_messenger();

    VkShaderModule create_shader_module(const std::vector<char>& code);

    void record_commandbuffer(VkCommandBuffer commandBuffer, U32 image_idx);

    Platform::Window* m_Window;

    std::unique_ptr<VkState> m_vkState;
    std::vector<const char*> m_ValidationLayers;
    std::vector<const char*> m_DeviceExtensions;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;

    std::vector<VkImage> m_SwapchainImages;
    VkFormat m_SwapchainImageFormat;
    VkExtent2D m_SwapchainExtent;

    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkSurfaceKHR m_Surface;
    VkSwapchainKHR m_Swapchain;
    VkRenderPass m_RenderPass;
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;
    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;

    VkSemaphore m_ImageAvailableSemaphore;
    VkSemaphore m_RenderFinishedSemaphore;
    VkFence m_InFlightFence;
};
}  // namespace Renderer
