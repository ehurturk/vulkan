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

    void create_instance();
    std::vector<const char*> getRequiredExtensions();
    void setup_debug_messenger();
    void destroy_debug_messenger();
    bool check_validation_layer_support();
    void pick_physical_device();
    bool is_physical_device_suitable(VkPhysicalDevice device);
    void create_logical_device();
    void create_surface();

    QueueFamilyIndices find_queue_families(VkPhysicalDevice device);

    Platform::Window* m_Window;

    std::unique_ptr<VkState> m_vkState;
    std::vector<const char*> m_ValidationLayers;

    VkDevice m_Device;
    VkPhysicalDevice m_PhysicalDevice;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkSurfaceKHR m_Surface;
};
}  // namespace Renderer
