#include "./platform.hpp"
#include "platform/core/PlatformContext.hpp"
#include "platform/window.hpp"
#include "../window/glfw_window.hpp"
#include "../window/headless_window.hpp"

namespace Platform {
UnixPlatform::UnixPlatform(const PlatformContext& context, UnixType type)
    : Platform(context), m_Type(type) {}

void UnixPlatform::createWindow(const Window::Properties& properties) {
    switch (properties.mode) {
        case Window::Mode::HEADLESS:
            m_Window = std::make_unique<HeadlessWindow>();
            break;
        default:
            m_Window = std::make_unique<GLFWWindow>();
            break;
    }
}
}  // namespace Platform