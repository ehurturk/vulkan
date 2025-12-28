#pragma once

#include "input_types.hpp"

namespace Platform {

enum class InputEventType : U8 {
    KeyPress,
    KeyRelease,
    KeyRepeat,
    MouseButtonPress,
    MouseButtonRelease,
    MouseMove,
    MouseScroll,
    MouseEnter,
    MouseLeave
};

struct KeyEvent {
    KeyCode key;
    KeyAction action;
    ModifierKey modifiers;
    I32 scancode;

    constexpr KeyEvent(KeyCode k, KeyAction a, ModifierKey m = ModifierKey::None, I32 s = 0)
        : key(k), action(a), modifiers(m), scancode(s) {}
};

struct MouseButtonEvent {
    MouseButton button;
    MouseAction action;
    ModifierKey modifiers;
    double xPos;
    double yPos;

    constexpr MouseButtonEvent(MouseButton b, MouseAction a, ModifierKey m, double x, double y)
        : button(b), action(a), modifiers(m), xPos(x), yPos(y) {}
};

struct MouseMoveEvent {
    double xPos;
    double yPos;
    double xDelta;
    double yDelta;

    constexpr MouseMoveEvent(double x, double y, double dx = 0.0, double dy = 0.0)
        : xPos(x), yPos(y), xDelta(dx), yDelta(dy) {}
};

struct MouseScrollEvent {
    double xOffset;
    double yOffset;

    constexpr MouseScrollEvent(double x, double y) : xOffset(x), yOffset(y) {}
};

struct MouseEnterEvent {
    bool entered;

    constexpr explicit MouseEnterEvent(bool e) : entered(e) {}
};

struct InputEvent {
    InputEventType type;

    union {
        KeyEvent key;
        MouseButtonEvent mouseButton;
        MouseMoveEvent mouseMove;
        MouseScrollEvent mouseScroll;
        MouseEnterEvent mouseEnter;
    };

    static InputEvent createKeyEvent(KeyCode keyCode,
                                     KeyAction action,
                                     ModifierKey mods = ModifierKey::None,
                                     I32 scancode = 0) {
        InputEvent event;
        event.type = action == KeyAction::Press     ? InputEventType::KeyPress
                     : action == KeyAction::Release ? InputEventType::KeyRelease
                                                    : InputEventType::KeyRepeat;
        event.key = KeyEvent{keyCode, action, mods, scancode};
        return event;
    }

    static InputEvent createMouseButtonEvent(MouseButton button,
                                             MouseAction action,
                                             ModifierKey mods,
                                             double x,
                                             double y) {
        InputEvent event;
        event.type = action == MouseAction::Press ? InputEventType::MouseButtonPress
                                                  : InputEventType::MouseButtonRelease;
        event.mouseButton = MouseButtonEvent{button, action, mods, x, y};
        return event;
    }

    static InputEvent createMouseMoveEvent(double x, double y, double dx = 0.0, double dy = 0.0) {
        InputEvent event;
        event.type = InputEventType::MouseMove;
        event.mouseMove = MouseMoveEvent{x, y, dx, dy};
        return event;
    }

    static InputEvent createMouseScrollEvent(double xOffset, double yOffset) {
        InputEvent event;
        event.type = InputEventType::MouseScroll;
        event.mouseScroll = MouseScrollEvent{xOffset, yOffset};
        return event;
    }

    static InputEvent createMouseEnterEvent(bool entered) {
        InputEvent event;
        event.type = entered ? InputEventType::MouseEnter : InputEventType::MouseLeave;
        event.mouseEnter = MouseEnterEvent{entered};
        return event;
    }

    InputEvent() : type(InputEventType::KeyPress), key(KeyCode::Unknown, KeyAction::Release) {}
};

}  // namespace Platform
