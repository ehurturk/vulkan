#pragma once

#include <functional>
#include <memory>
#include <span>
#include <utility>

#include "input_events.hpp"
#include "input_types.hpp"

namespace Platform {

class Window;

using KeyCallback = std::function<void(const KeyEvent&)>;
using MouseButtonCallback = std::function<void(const MouseButtonEvent&)>;
using MouseMoveCallback = std::function<void(const MouseMoveEvent&)>;
using MouseScrollCallback = std::function<void(const MouseScrollEvent&)>;
using MouseEnterCallback = std::function<void(const MouseEnterEvent&)>;

class Input {
   public:
    virtual ~Input() = default;

    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual bool isMouseButtonPressed(MouseButton button) const = 0;
    virtual std::pair<double, double> getMousePosition() const = 0;
    virtual std::pair<double, double> getMouseDelta() const = 0;

    virtual void setCursorMode(bool visible, bool confined = false) = 0;
    virtual void setCursorPosition(double x, double y) = 0;

    virtual void setKeyCallback(KeyCallback callback) = 0;
    virtual void setMouseButtonCallback(MouseButtonCallback callback) = 0;
    virtual void setMouseMoveCallback(MouseMoveCallback callback) = 0;
    virtual void setMouseScrollCallback(MouseScrollCallback callback) = 0;
    virtual void setMouseEnterCallback(MouseEnterCallback callback) = 0;

    virtual void processEvents() = 0;

    virtual std::span<const InputEvent> getFrameEvents() const = 0;
    virtual void clearFrameEvents() = 0;

   protected:
    Input() = default;
};

std::unique_ptr<Input> CreateInput(Window& window);

}  // namespace Platform
