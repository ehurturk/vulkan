#include "platform/window/headless_window.hpp"

namespace Platform {

HeadlessWindow::HeadlessWindow()
    : Window(Properties{})  // or pass proper default properties
{
    // Initialize GLFW window
}

HeadlessWindow::~HeadlessWindow() {
    // Cleanup
}

// Implement all the pure virtual functions from Window base class
VkSurfaceKHR HeadlessWindow::createSurface(VkInstance instance) {
    // TODO: Implement
    (void)instance;
    return VK_NULL_HANDLE;
}

void HeadlessWindow::processEvents() {
    // TODO: Implement
}

void HeadlessWindow::waitForEvents() {
    // TODO: implement
}

void HeadlessWindow::close() {
    // TODO: Implement
}

bool HeadlessWindow::shouldClose() {
    // TODO: Implement
    return false;
}

float HeadlessWindow::getDPI() const {
    // TODO: Implement
    return 96.0f;
}

float HeadlessWindow::getContentScaleFactor() const {
    // TODO: Implement
    return 1.0f;
}

const HeadlessWindow::Extent HeadlessWindow::getExtentPixel() const {
    return {.width = 0, .height = 0};
}

Window::Extent HeadlessWindow::getFramebufferSize() const {
    return {.width = 0, .height = 0};
}

std::vector<const char*> HeadlessWindow::getRequiredInstanceExtensions() const {
    // TODO: Implement
    return {};
}

}  // namespace Platform
