#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <memory>

#include "renderer/backend/renderer.hpp"

#include <vector>

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

    void create_instance();
    std::vector<const char*> getRequiredExtensions();
    void setup_debug_messenger();
    void destroy_debug_messenger();
    bool check_validation_layer_support();

    std::unique_ptr<VkState> m_vkState;
    std::vector<const char*> m_ValidationLayers;
};
}  // namespace Renderer
