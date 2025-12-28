#include "glfw_input.hpp"

#include <GLFW/glfw3.h>

#include "core/assert.hpp"
#include "platform/window/window.hpp"
#include "platform/window/glfw_window.hpp"

namespace Platform {

GLFWInput::GLFWInput(Window& window) : m_WindowHandle(nullptr) {
    auto* glfwWindow = dynamic_cast<GLFWWindow*>(&window);
    ASSERT_MSG(glfwWindow != nullptr, "Window must be a GLFWWindow");

    m_WindowHandle = glfwWindow->getNativeHandle();
    ASSERT_MSG(m_WindowHandle != nullptr, "GLFW window handle is null");

    glfwSetWindowUserPointer(m_WindowHandle, this);

    glfwSetKeyCallback(m_WindowHandle, glfwKeyCallback);
    glfwSetMouseButtonCallback(m_WindowHandle, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(m_WindowHandle, glfwCursorPosCallback);
    glfwSetScrollCallback(m_WindowHandle, glfwScrollCallback);
    glfwSetCursorEnterCallback(m_WindowHandle, glfwCursorEnterCallback);

    m_FrameEvents.reserve(128);
}

GLFWInput::~GLFWInput() {
    if (m_WindowHandle) {
        glfwSetKeyCallback(m_WindowHandle, nullptr);
        glfwSetMouseButtonCallback(m_WindowHandle, nullptr);
        glfwSetCursorPosCallback(m_WindowHandle, nullptr);
        glfwSetScrollCallback(m_WindowHandle, nullptr);
        glfwSetCursorEnterCallback(m_WindowHandle, nullptr);
        glfwSetWindowUserPointer(m_WindowHandle, nullptr);
    }
}

bool GLFWInput::isKeyPressed(KeyCode key) const {
    if (!m_WindowHandle)
        return false;

    int glfwKey = static_cast<int>(key);
    int state = glfwGetKey(m_WindowHandle, glfwKey);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool GLFWInput::isMouseButtonPressed(MouseButton button) const {
    if (!m_WindowHandle)
        return false;

    int glfwButton = static_cast<int>(button);
    int state = glfwGetMouseButton(m_WindowHandle, glfwButton);
    return state == GLFW_PRESS;
}

std::pair<double, double> GLFWInput::getMousePosition() const {
    if (!m_WindowHandle)
        return {0.0, 0.0};

    double x, y;
    glfwGetCursorPos(m_WindowHandle, &x, &y);
    return {x, y};
}

std::pair<double, double> GLFWInput::getMouseDelta() const {
    auto [currentX, currentY] = getMousePosition();
    if (m_FirstMouseInput) {
        return {0.0, 0.0};
    }
    return {currentX - m_LastMouseX, currentY - m_LastMouseY};
}

void GLFWInput::setCursorMode(bool visible, bool confined) {
    if (!m_WindowHandle)
        return;

    int mode = GLFW_CURSOR_NORMAL;
    if (!visible && confined) {
        mode = GLFW_CURSOR_DISABLED;
    } else if (!visible) {
        mode = GLFW_CURSOR_HIDDEN;
    }

    glfwSetInputMode(m_WindowHandle, GLFW_CURSOR, mode);
}

void GLFWInput::setCursorPosition(double x, double y) {
    if (!m_WindowHandle)
        return;
    glfwSetCursorPos(m_WindowHandle, x, y);
}

void GLFWInput::setKeyCallback(KeyCallback callback) {
    m_KeyCallback = std::move(callback);
}

void GLFWInput::setMouseButtonCallback(MouseButtonCallback callback) {
    m_MouseButtonCallback = std::move(callback);
}

void GLFWInput::setMouseMoveCallback(MouseMoveCallback callback) {
    m_MouseMoveCallback = std::move(callback);
}

void GLFWInput::setMouseScrollCallback(MouseScrollCallback callback) {
    m_ScrollCallback = std::move(callback);
}

void GLFWInput::setMouseEnterCallback(MouseEnterCallback callback) {
    m_EnterCallback = std::move(callback);
}

void GLFWInput::processEvents() {
    // Events are processed by GLFW callbacks automatically
    // This is called after glfwPollEvents() in the platform layer
}

std::span<const InputEvent> GLFWInput::getFrameEvents() const {
    return m_FrameEvents;
}

void GLFWInput::clearFrameEvents() {
    m_FrameEvents.clear();
}

void GLFWInput::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* input = static_cast<GLFWInput*>(glfwGetWindowUserPointer(window));
    if (!input)
        return;

    KeyCode keyCode = glfwKeyToKeyCode(key);
    KeyAction keyAction = static_cast<KeyAction>(action);
    ModifierKey modifiers = glfwModsToModifierKey(mods);

    KeyEvent event{keyCode, keyAction, modifiers, scancode};

    input->m_FrameEvents.push_back(
        InputEvent::createKeyEvent(keyCode, keyAction, modifiers, scancode));

    if (input->m_KeyCallback) {
        input->m_KeyCallback(event);
    }
}

void GLFWInput::glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* input = static_cast<GLFWInput*>(glfwGetWindowUserPointer(window));
    if (!input)
        return;

    MouseButton mouseButton = glfwMouseButtonToMouseButton(button);
    MouseAction mouseAction = static_cast<MouseAction>(action);
    ModifierKey modifiers = glfwModsToModifierKey(mods);

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    MouseButtonEvent event{mouseButton, mouseAction, modifiers, x, y};

    input->m_FrameEvents.push_back(
        InputEvent::createMouseButtonEvent(mouseButton, mouseAction, modifiers, x, y));

    if (input->m_MouseButtonCallback) {
        input->m_MouseButtonCallback(event);
    }
}

void GLFWInput::glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* input = static_cast<GLFWInput*>(glfwGetWindowUserPointer(window));
    if (!input)
        return;

    double dx = 0.0;
    double dy = 0.0;

    if (!input->m_FirstMouseInput) {
        dx = xpos - input->m_LastMouseX;
        dy = ypos - input->m_LastMouseY;
    }

    input->m_FirstMouseInput = false;
    input->m_LastMouseX = xpos;
    input->m_LastMouseY = ypos;

    MouseMoveEvent event{xpos, ypos, dx, dy};

    input->m_FrameEvents.push_back(InputEvent::createMouseMoveEvent(xpos, ypos, dx, dy));

    if (input->m_MouseMoveCallback) {
        input->m_MouseMoveCallback(event);
    }
}

void GLFWInput::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* input = static_cast<GLFWInput*>(glfwGetWindowUserPointer(window));
    if (!input)
        return;

    MouseScrollEvent event{xoffset, yoffset};

    input->m_FrameEvents.push_back(InputEvent::createMouseScrollEvent(xoffset, yoffset));

    if (input->m_ScrollCallback) {
        input->m_ScrollCallback(event);
    }
}

void GLFWInput::glfwCursorEnterCallback(GLFWwindow* window, int entered) {
    auto* input = static_cast<GLFWInput*>(glfwGetWindowUserPointer(window));
    if (!input)
        return;

    bool hasEntered = entered == GLFW_TRUE;
    MouseEnterEvent event{hasEntered};

    input->m_FrameEvents.push_back(InputEvent::createMouseEnterEvent(hasEntered));

    if (input->m_EnterCallback) {
        input->m_EnterCallback(event);
    }
}

KeyCode GLFWInput::glfwKeyToKeyCode(int glfwKey) {
    if (glfwKey >= 0 && glfwKey <= 348) {
        return static_cast<KeyCode>(glfwKey);
    }
    return KeyCode::Unknown;
}

MouseButton GLFWInput::glfwMouseButtonToMouseButton(int glfwButton) {
    if (glfwButton >= GLFW_MOUSE_BUTTON_1 && glfwButton <= GLFW_MOUSE_BUTTON_8) {
        return static_cast<MouseButton>(glfwButton);
    }
    return MouseButton::Unknown;
}

ModifierKey GLFWInput::glfwModsToModifierKey(int glfwMods) {
    ModifierKey mods = ModifierKey::None;

    if (glfwMods & GLFW_MOD_SHIFT)
        mods = mods | ModifierKey::Shift;
    if (glfwMods & GLFW_MOD_CONTROL)
        mods = mods | ModifierKey::Control;
    if (glfwMods & GLFW_MOD_ALT)
        mods = mods | ModifierKey::Alt;
    if (glfwMods & GLFW_MOD_SUPER)
        mods = mods | ModifierKey::Super;
    if (glfwMods & GLFW_MOD_CAPS_LOCK)
        mods = mods | ModifierKey::CapsLock;
    if (glfwMods & GLFW_MOD_NUM_LOCK)
        mods = mods | ModifierKey::NumLock;

    return mods;
}

std::unique_ptr<Input> CreateInput(Window& window) {
    // TODO: Add platform checks if multiple platforms are supported
    return std::make_unique<GLFWInput>(window);
}

}  // namespace Platform
