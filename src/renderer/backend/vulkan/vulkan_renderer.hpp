#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Renderer {

class Renderer;

class VulkanRenderer {
  public:
    VulkanRenderer();
    ~VulkanRenderer();

    void initialize(Renderer *renderer);
    void destroy();

  private:
    struct InternalState {
        VkInstance instance = VK_NULL_HANDLE;
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};
    };

    void createInstance();
    void handleExtensions(std::vector<const char *> &extensions);

    std::unique_ptr<InternalState> m_state;
    Renderer *m_renderer = nullptr;
};

}
