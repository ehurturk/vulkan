#include "window.hpp"
#include "../core/logger.hpp"
#include <GLFW/glfw3.h>

namespace Platform {

B8 Window::s_glfwInitialized = false;

Window::Window(const Config& config) : m_config(config) {
    LOG_INFO("Creating window {}...", m_config.name);

    if (!s_glfwInitialized) {
        if (!glfwInit()) {
            LOG_FATAL("Failed to initialize GLFW.");
            throw std::runtime_error("GLFW initialization failed");
        }
        s_glfwInitialized = true;
    }
}

Window::~Window() {
    shutdown();
}

B8 Window::create() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, m_config.resizable ? GLFW_TRUE : GLFW_FALSE);

    if (!m_config.fullscreen) {
        m_handle = glfwCreateWindow(m_config.width, m_config.height, m_config.name.c_str(), nullptr,
                                    nullptr);
    } else {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        m_handle = glfwCreateWindow(m_config.width, m_config.height, m_config.name.c_str(), primary,
                                    nullptr);
    }

    if (!m_handle) {
        LOG_FATAL("Failed to create GLFW window.");
        return false;
    }

    glfwSetWindowUserPointer(m_handle, this);
    return true;
}

void Window::pollEvents() {
    glfwPollEvents();
}

B8 Window::shouldClose() const {
    return m_handle ? glfwWindowShouldClose(m_handle) : true;
}

void Window::shutdown() {
    if (m_handle) {
        LOG_INFO("Destroying window {}...", m_config.name);
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
    }
}

}  // namespace Platform