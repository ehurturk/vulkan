#pragma once

#include "input.hpp"

#include <vector>

struct GLFWwindow;

namespace Platform {

class GLFWInput final : public Input {
   public:
    explicit GLFWInput(Window& window);
    ~GLFWInput() override;

    GLFWInput(const GLFWInput&) = delete;
    GLFWInput& operator=(const GLFWInput&) = delete;

    GLFWInput(GLFWInput&&) noexcept = default;
    GLFWInput& operator=(GLFWInput&&) noexcept = default;

    bool isKeyPressed(KeyCode key) const override;
    bool isMouseButtonPressed(MouseButton button) const override;
    std::pair<double, double> getMousePosition() const override;
    std::pair<double, double> getMouseDelta() const override;

    void setCursorMode(bool visible, bool confined) override;
    void setCursorPosition(double x, double y) override;

    void setKeyCallback(KeyCallback callback) override;
    void setMouseButtonCallback(MouseButtonCallback callback) override;
    void setMouseMoveCallback(MouseMoveCallback callback) override;
    void setMouseScrollCallback(MouseScrollCallback callback) override;
    void setMouseEnterCallback(MouseEnterCallback callback) override;

    void processEvents() override;
    std::span<const InputEvent> getFrameEvents() const override;
    void clearFrameEvents() override;

   private:
    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void glfwCursorEnterCallback(GLFWwindow* window, int entered);

    static KeyCode glfwKeyToKeyCode(int glfwKey);
    static MouseButton glfwMouseButtonToMouseButton(int glfwButton);
    static ModifierKey glfwModsToModifierKey(int glfwMods);

    GLFWwindow* m_WindowHandle;

    KeyCallback m_KeyCallback;
    MouseButtonCallback m_MouseButtonCallback;
    MouseMoveCallback m_MouseMoveCallback;
    MouseScrollCallback m_ScrollCallback;
    MouseEnterCallback m_EnterCallback;

    std::vector<InputEvent> m_FrameEvents;

    double m_LastMouseX{0.0};
    double m_LastMouseY{0.0};
    bool m_FirstMouseInput{true};
};

}  // namespace Platform
