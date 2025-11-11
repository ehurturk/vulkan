#include "glfw_window.hpp"

#include "core/logger.hpp"

namespace Platform {

GLFWWindow::GLFWWindow(const Window::Properties& properties) : Window(properties) {
    LOG_INFO("Creating GLFW Window {}...", m_Properties.title);
    glfwInit();

    // Initialize GLFW window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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

// Implement all the pure virtual functions from Window base class
VkSurfaceKHR GLFWWindow::createSurface(VkInstance instance, VkPhysicalDevice physicalDevice) {
    // TODO: Implement
    (void)instance;
    (void)physicalDevice;
    return VK_NULL_HANDLE;
}

void GLFWWindow::processEvents() {
    glfwPollEvents();
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

std::vector<const char*> GLFWWindow::getRequiredSurfaceExtensions() const {
    // TODO: Implement
    std::vector<const char*> vec;
    return vec;
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