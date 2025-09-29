#pragma once

#include <vulkan/vulkan.h>
#include "platform/window.hpp"

namespace Platform {
class GLFWWindow : public Window {
   public:
    GLFWWindow();
    ~GLFWWindow();

    VkSurfaceKHR createSurface(VkInstance instance,
                               VkPhysicalDevice physicalDevice = VK_NULL_HANDLE) override;

    void processEvents() override;

    void close() override;
    bool shouldClose() override;

    float getDPI() const override;
    float getContentScaleFactor() const override;

    std::vector<const char*> getRequiredSurfaceExtensions() const override;
};
}  // namespace Platform