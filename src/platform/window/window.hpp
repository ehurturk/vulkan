#pragma once

#include "defines.hpp"

#include <optional>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace Platform {

class Window {
   public:
    struct Extent {
        U32 width;
        U32 height;
    };

    struct OptionalExtent {
        std::optional<U32> width;
        std::optional<U32> height;
    };

    enum class Mode { HEADLESS, FULLSCREEN, FULLSCREEN_BORDERLESS, FULLSCREEN_STRETCH, DEFAULT };
    enum class Vsync { OFF, ON, DEFAULT };

    struct OptionalProperties {
        std::optional<std::string> title;
        std::optional<Mode> mode;
        std::optional<bool> resizable;
        std::optional<Vsync> vsync;
        OptionalExtent extent;
    };

    struct Properties {
        std::string title = "";
        Mode mode = Mode::DEFAULT;
        bool resizable = true;
        Vsync vsync = Vsync::DEFAULT;
        Extent extent = {1280, 720};
    };

    explicit Window(const Properties& properties);
    virtual ~Window() = default;

    /* Vulkan Surface */
    virtual VkSurfaceKHR createSurface(
        VkInstance instance) = 0;  // TODO: Abstract the surface into a API-Agnostic surface type?
    virtual std::vector<const char*> getRequiredInstanceExtensions() const = 0;

    virtual void processEvents() = 0;
    virtual void waitForEvents() = 0;

    virtual void close() = 0;
    virtual bool shouldClose() = 0;

    virtual float getDPI() const = 0;
    virtual float getContentScaleFactor() const;

    virtual void setTitle(const std::string& title) { m_Properties.title = title; }
    std::string getTitle() const { return m_Properties.title; }

    Extent resize(const Extent& extent);

    // TODO: Graphics API abstraction for VkDisplayPresentInfoKHR
    virtual bool getDisplayPresentInfo(VkDisplayPresentInfoKHR* info,
                                       U32 src_width,
                                       U32 src_height) const;

    const Extent& getExtent() const;
    virtual const Extent getExtentPixel() const = 0;

    virtual Extent getFramebufferSize() const = 0;

    Mode getWindowMode() const;

    const Properties& getProperties() const { return m_Properties; }

   protected:
    Properties m_Properties;
};
}  // namespace Platform
