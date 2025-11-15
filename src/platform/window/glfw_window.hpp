#pragma once

#include <vulkan/vulkan.h>
#include "platform/window.hpp"

#include <vector>

// forward declare
struct GLFWwindow;

namespace Platform {
class GLFWWindow : public Window {
   public:
    GLFWWindow(const Window::Properties& properties);
    ~GLFWWindow();

    VkSurfaceKHR createSurface(VkInstance instance) override;

    void processEvents() override;

    void close() override;
    bool shouldClose() override;

    float getDPI() const override;
    float getContentScaleFactor() const override;

    std::vector<const char*> getRequiredInstanceExtensions() const override;

    bool getDisplayPresentInfo(VkDisplayPresentInfoKHR* info,
                               U32 src_width,
                               U32 src_height) const override;

    void setTitle(const std::string& title) override;

   private:
    GLFWwindow* m_Window;
};
}  // namespace Platform
