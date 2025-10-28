#include "glfw_window.hpp"

namespace Platform {

GLFWWindow::GLFWWindow()
    : Window(Properties{})  // or pass proper default properties
{
    // Initialize GLFW window
}

GLFWWindow::~GLFWWindow() {
    // Cleanup
}

// Implement all the pure virtual functions from Window base class
VkSurfaceKHR GLFWWindow::createSurface(VkInstance instance, VkPhysicalDevice physicalDevice) {
    // TODO: Implement
    (void)instance;
    (void)physicalDevice;
    return VK_NULL_HANDLE;
}

void GLFWWindow::processEvents() {
    // TODO: Implement
}

void GLFWWindow::close() {
    // TODO: Implement
}

bool GLFWWindow::shouldClose() {
    // TODO: Implement
    return false;
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

}  // namespace Platform
