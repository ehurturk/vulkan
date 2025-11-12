#pragma once

#include <memory>
#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "renderer/backend/renderer.hpp"
#include "defines.hpp"

namespace Renderer {

class VulkanRenderer final : public RendererBackend {
   public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    void initialize(const RendererConfig& cfg) override;
    void shutdown() override;

   private:
    struct VkState {
        VkInstance instance = VK_NULL_HANDLE;
        VkInstanceCreateInfo createInfo;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        bool validation = false;
    };

    struct QueueFamilyIndices {
        std::optional<U32> graphicsFamily;

        bool is_complete() { return graphicsFamily.has_value(); }
    };

    void create_instance();
    std::vector<const char*> getRequiredExtensions();
    void setup_debug_messenger();
    void destroy_debug_messenger();
    bool check_validation_layer_support();
    void pick_physical_device();
    bool is_physical_device_suitable(VkPhysicalDevice device);
    void create_logical_device();

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

    std::unique_ptr<VkState> m_vkState;
    std::vector<const char*> m_ValidationLayers;

    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    VkQueue m_GraphicsQueue;
};
}  // namespace Renderer
