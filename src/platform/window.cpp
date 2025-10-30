#include "window.hpp"

namespace Platform {

Window::Window(const Properties& properties) : m_Properties{properties} {}

void Window::processEvents() {}

Window::Extent Window::resize(const Extent& new_extent) {
    if (m_Properties.resizable) {
        m_Properties.extent.width = new_extent.width;
        m_Properties.extent.height = new_extent.height;
    }

    return m_Properties.extent;
}

const Window::Extent& Window::getExtent() const {
    return m_Properties.extent;
}

float Window::getContentScaleFactor() const {
    return 1.0f;
}

Window::Mode Window::getWindowMode() const {
    return m_Properties.mode;
}

bool Window::getDisplayPresentInfo([[maybe_unused]] VkDisplayPresentInfoKHR* info,
                                   [[maybe_unused]] uint32_t src_width,
                                   [[maybe_unused]] uint32_t src_height) const {
    // base Window class will not use any extra present info
    return false;
}
}  // namespace Platform
