#pragma once

#include <vulkan/vulkan.h>
#include "platform/window.hpp"

#include <vector>

namespace Platform {
class HeadlessWindow : public Window {
   public:
    HeadlessWindow();
    ~HeadlessWindow();

    VkSurfaceKHR createSurface(VkInstance instance) override;

    void processEvents() override;

    void close() override;
    bool shouldClose() override;

    float getDPI() const override;
    float getContentScaleFactor() const override;

    std::vector<const char*> getRequiredInstanceExtensions() const override;
};
}  // namespace Platform
