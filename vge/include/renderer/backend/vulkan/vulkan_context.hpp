#pragma once

#include "defines.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>

namespace Platform {
class Window;
}

namespace Renderer::Vulkan {

struct VulkanConfiguration {
    U32 VkApiVersion = VK_MAKE_VERSION(1, 3, 0);
    bool ValidationLayersEnabled = TRUE_IF_DEBUG;
};

// Vulkan Graphics Context
class VulkanContext {
public:
    VulkanContext(Platform::Window& window, VulkanConfiguration config = VulkanConfiguration {});
    ~VulkanContext();

    // Initializes the vulkan context, creating the VkInstance and VkSurface handles,
    // setting up debug messengers and enabling validation layers (if supported).
    void initialize();

    // Destroys the vulkan context, freeing up resources such as the instance and the
    // window surface.
    void destroy();

    inline void set_validations_to(bool enable) { m_Config.ValidationLayersEnabled = enable; }
    inline void enable_validations() { m_Config.ValidationLayersEnabled = true; }
    inline void disable_validations() { m_Config.ValidationLayersEnabled = false; }
    inline bool validations_enabled() const { return m_Config.ValidationLayersEnabled; }

    inline VkInstance vk_instance() const { return m_Instance; }
    inline VkSurfaceKHR vk_surface() const { return m_Surface; }
    inline VkDebugUtilsMessengerEXT vk_debug_messenger() const { return m_DebugMessenger; }
    inline U32 vk_version() const { return m_Config.VkApiVersion; }

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

private:
    Platform::Window& m_Window;

    VkInstance m_Instance { VK_NULL_HANDLE };
    VkSurfaceKHR m_Surface { VK_NULL_HANDLE };

    VkDebugUtilsMessengerEXT m_DebugMessenger { VK_NULL_HANDLE };

    std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

    VulkanConfiguration m_Config;

    void create_instance();
    void create_surface();

    void setup_debug_messenger();

    bool check_validation_layer_support();
    std::vector<const char*> get_required_extensions() const;
};

using GraphicsContext = VulkanContext;

} // namespace Renderer::Vulkan
