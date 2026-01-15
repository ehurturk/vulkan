#pragma once

#include <vulkan/vulkan.h>
#include "renderer/backend/vulkan/vulkan_context.hpp"
#include "window.hpp"

#include <vector>

// forward declare
struct GLFWwindow;

namespace Platform {
class GLFWWindow final : public Window {
public:
    explicit GLFWWindow(const Window::Properties& properties);
    ~GLFWWindow() override;

    VkSurfaceKHR createSurface(Renderer::Vulkan::GraphicsContext& instance) override;

    void processEvents() override;
    void waitForEvents() override;

    void close() override;
    bool shouldClose() override;

    float getDPI() const override;
    float getContentScaleFactor() const override;

    const Extent getExtentPixel() const override;

    Extent getFramebufferSize() const override;

    std::vector<const char*> getRequiredInstanceExtensions() const override;

    bool getDisplayPresentInfo(
        VkDisplayPresentInfoKHR* info, U32 src_width, U32 src_height) const override;

    void setTitle(const std::string& title) override;

    // Get native window handle for input system
    GLFWwindow* getNativeHandle() const { return m_Window; }

private:
    GLFWwindow* m_Window;
};
} // namespace Platform
