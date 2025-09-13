#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "renderer/backend/renderer.hpp"

namespace Renderer {

class VulkanRenderer final : public IRendererBackend {
  public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    void initialize(const RendererConfig &cfg) override;
    void shutdown() override;

  private:
    struct VkState {
        VkInstance instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
        bool validation = false;
    };

    void create_instance(bool enableValidation);
    void setup_debug_messenger();
    void destroy_debug_messenger();

    std::unique_ptr<VkState> m_vkState;
};
} // namespace Renderer
