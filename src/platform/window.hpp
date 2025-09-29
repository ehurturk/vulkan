#pragma once

#include "defines.hpp"

#include <optional>
#include <vulkan/vulkan.h>
#include <string>

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

    Window(const Properties& properties);

    virtual ~Window() = default;

    virtual VkSurfaceKHR create_surface(VkInstance instance,
                                        VkPhysicalDevice physical_device = VK_NULL_HANDLE) = 0;

    virtual bool should_close() = 0;

    virtual void process_events();

    virtual void close() = 0;

    virtual float get_dpi() const = 0;

    virtual float get_content_scale_factor() const;

    Extent resize(const Extent& extent);

    virtual bool get_display_present_info(VkDisplayPresentInfoKHR* info,
                                          U32 src_width,
                                          U32 src_height) const;

    virtual std::vector<const char*> get_required_surface_extensions() const = 0;

    const Extent& get_extent() const;

    Mode get_window_mode() const;

    inline const Properties& get_properties() const { return properties; }

   protected:
    Properties properties;
};
}  // namespace Platform