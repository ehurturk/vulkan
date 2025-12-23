#pragma once

#include <vulkan/vulkan.h>
#include "platform/window/window.hpp"

#include <vector>

namespace Platform {
class HeadlessWindow final : public Window {
   public:
    HeadlessWindow();
    ~HeadlessWindow() override;

    VkSurfaceKHR createSurface(VkInstance instance) override;

    void processEvents() override;
    void waitForEvents() override;

    void close() override;
    bool shouldClose() override;

    const Extent getExtentPixel() const override;

    Extent getFramebufferSize() const override;

    float getDPI() const override;
    float getContentScaleFactor() const override;

    std::vector<const char*> getRequiredInstanceExtensions() const override;
};
}  // namespace Platform
