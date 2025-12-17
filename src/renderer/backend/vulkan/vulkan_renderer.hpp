#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "renderer/backend/renderer.hpp"
#include "defines.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

namespace Platform {
class Window;
}

namespace Renderer::Vulkan {

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription get_binding_description() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.stride = sizeof(Vertex);

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> get_attribute_description() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

// Alignment Requirements:
// float = 4 bytes
// glm::vec2 = 8 bytes
// glm::vec3/glm::vec4 = 16 bytes
// glm::mat4 = glm::vec4 =16 bytes
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
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

        bool is_complete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    std::vector<const char*> getRequiredExtensions() const;

    void create_instance();
    void setup_debug_messenger();
    void create_surface();
    void pick_physical_device();
    void create_logical_device();
    void create_swapchain();
    void create_image_views();
    void create_descriptor_set_layout();
    void create_graphics_pipeline();
    void create_renderpass();
    void create_framebuffers();
    void create_commandpool();
    void create_texture_image();
    void create_texture_image_view();
    void create_texture_sampler();
    void create_vertex_buffer();
    void create_index_buffer();
    void create_uniform_buffers();
    void create_descriptor_pool();
    void create_descriptor_sets();
    void create_commandbuffers();
    void create_sync_objects();

    bool is_physical_device_suitable(VkPhysicalDevice device);

    // TODO: cache the result into a member variable?
    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
    bool check_device_extension_support(VkPhysicalDevice device);

    SwapchainSupportDetails query_swap_chain_support(VkPhysicalDevice device) const;

    VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

    bool check_validation_layer_support();

    VkShaderModule create_shader_module(const std::vector<char>& code) const;

    void record_draw_commands(VkCommandBuffer commandBuffer, U32 image_idx) const;

    void create_buffer(VkDeviceSize size,
                       VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       VkBuffer& buffer,
                       VkDeviceMemory& bufferMemory) const;

    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer commandBuffer);

    void copy_buffer(VkBuffer src, VkBuffer dest, VkDeviceSize size);

    void update_uniform_buffer(U32 imageIdx) const;

    VkImageView create_image_view(VkImage image, VkFormat format);

    void create_image(uint32_t width,
                      uint32_t height,
                      VkFormat format,
                      VkImageTiling tiling,
                      VkImageUsageFlags usage,
                      VkMemoryPropertyFlags properties,
                      VkImage& image,
                      VkDeviceMemory& imageMemory);

    void transition_image_layout(VkImage image,
                                 VkFormat format,
                                 VkImageLayout oldLayout,
                                 VkImageLayout newLayout);

    void copy_buffer_to_image(VkBuffer buffer, VkImage image, U32 width, U32 height);

    U32 find_memory_type(U32 typeFilter, VkMemoryPropertyFlags properties) const;

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

    VkDescriptorSetLayout m_DescriptorSetLayout;

    // TODO: Abstract away command pool & command buffer to
    // FrameData -> since only one thread at a time can record
    // commands, for multithreaded command recording we will
    // pair command buffer with its command allocator.
    VkCommandPool m_CommandPool;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_CommandBuffers;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;

    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexBufferMemory;

    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    std::vector<void*> m_UniformBuffersMapped;

    VkDescriptorPool m_DescriptorPool;
    std::array<VkDescriptorSet, MAX_FRAMES_IN_FLIGHT> m_DescriptorSets;

    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;

    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_InFlightFences;
};
}  // namespace Renderer::Vulkan
