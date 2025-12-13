#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "renderer/backend/renderer.hpp"
#include "defines.hpp"

#include <glm/glm.hpp>

namespace Platform {
class Window;
}

namespace Renderer {

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static VkVertexInputBindingDescription get_binding_description() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> get_attribute_description() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescription{};

        attributeDescription[0].binding = 0;
        attributeDescription[0].location = 0;
        attributeDescription[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescription[0].offset = offsetof(Vertex, position);

        attributeDescription[1].binding = 0;
        attributeDescription[1].location = 1;
        attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescription[1].offset = offsetof(Vertex, color);

        return attributeDescription;
    }
};

class VulkanRenderer final : public RendererBackend {
   public:
    VulkanRenderer(Platform::Window* window);
    ~VulkanRenderer() override;

    void initialize(const RendererConfig& cfg) override;
    void shutdown() override;
    void draw_frame() override;

   private:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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
    void create_vertex_buffer();
    void create_commandbuffers();
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

    void create_buffer(VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkBuffer& buffer,
                       VkDeviceMemory& bufferMemory);
    void copy_buffer(VkBuffer src, VkBuffer dest, VkDeviceSize size);

    U32 find_memory_type(U32 typeFilter, VkMemoryPropertyFlags properties);

    Platform::Window* m_Window;

    U32 m_CurrentFrame;

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
    std::array<VkCommandBuffer, VulkanRenderer::MAX_FRAMES_IN_FLIGHT> m_CommandBuffers;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    std::array<VkFence, VulkanRenderer::MAX_FRAMES_IN_FLIGHT> m_InFlightFences;
};
}  // namespace Renderer
