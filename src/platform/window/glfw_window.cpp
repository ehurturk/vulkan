#include "glfw_window.hpp"
#include <vulkan/vulkan_core.h>

#include "core/logger.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Platform {

GLFWWindow::GLFWWindow(const Window::Properties& properties) : Window(properties) {
    LOG_INFO("Creating GLFW Window...");
    glfwInit();

    // Initialize GLFW window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, properties.resizable);

    // GLFWmonitor* primary = glfwGetPrimaryMonitor();
    m_Window = glfwCreateWindow(m_Properties.extent.width, m_Properties.extent.height,
                                m_Properties.title.c_str(), nullptr, nullptr);

    if (!m_Window) {
        LOG_FATAL("Failed to create a GLFW window!");
        // throw std::runtime_error("GLFWWindowCreateError");
    }

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetWindowTitle(m_Window, properties.title.c_str());
}

GLFWWindow::~GLFWWindow() {
    // Cleanup
    close();
}

// for vulkan simply use glfw surface
VkSurfaceKHR GLFWWindow::createSurface(VkInstance instance) {
    VkSurfaceKHR surface{};
    if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface))
        return VK_NULL_HANDLE;
    return surface;
}

void GLFWWindow::processEvents() {
    glfwPollEvents();
}
void GLFWWindow::waitForEvents() {
    glfwWaitEvents();
}

void GLFWWindow::close() {
    glfwDestroyWindow(m_Window);
    glfwTerminate();
    m_Window = nullptr;
}

bool GLFWWindow::shouldClose() {
    // TODO: Implement
    return glfwWindowShouldClose(m_Window);
}

float GLFWWindow::getDPI() const {
    // TODO: Implement
    return 96.0f;
}

float GLFWWindow::getContentScaleFactor() const {
    // TODO: Implement
    return 1.0f;
}

const GLFWWindow::Extent GLFWWindow::getExtentPixel() const {
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);
    return {.width = static_cast<U32>(width), .height = static_cast<U32>(height)};
}

Window::Extent GLFWWindow::getFramebufferSize() const {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_Window, &width, &height);

    return {.width = static_cast<U32>(width), .height = static_cast<U32>(height)};
}

std::vector<const char*> GLFWWindow::getRequiredInstanceExtensions() const {
    uint32_t glfwCount = 0;
    const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwCount);

    return {glfwExts, glfwExts + glfwCount};
}

bool GLFWWindow::getDisplayPresentInfo(VkDisplayPresentInfoKHR* info,
                                       U32 src_width,
                                       U32 src_height) const {
    (void)info;
    (void)src_width;
    (void)src_height;

    return true;
}

void GLFWWindow::setTitle(const std::string& title) {
    m_Properties.title = title;
    glfwSetWindowTitle(m_Window, title.c_str());
}

}  // namespace Platform
