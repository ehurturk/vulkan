#include "./platform.hpp"
#include "core/logger.hpp"
#include "platform/core/PlatformContext.hpp"
#include "platform/window.hpp"
#include "../window/glfw_window.hpp"
#include "../window/headless_window.hpp"

namespace Platform {
UnixPlatform::UnixPlatform(const PlatformContext& context, UnixType type)
    : Platform(context), m_Type(type) {
    switch (m_Type) {
        case UnixType::MACOS:
            LOG_INFO("Using UNIX Type: MACOS");
            break;
        case UnixType::LINUX:
            LOG_INFO("Using UNIX Type: LINUX");
            break;
    }
}

UnixPlatform::~UnixPlatform() {
    // TODO: Cleanup
}

void UnixPlatform::createWindow(const Window::Properties& properties) {
    switch (properties.mode) {
        case Window::Mode::HEADLESS:
            m_Window = std::make_unique<HeadlessWindow>();
            break;
        default:
            m_Window = std::make_unique<GLFWWindow>(properties);
            break;
    }
}
}  // namespace Platform
