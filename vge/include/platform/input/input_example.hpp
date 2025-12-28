#pragma once

/**
 * Example usage of the Input System
 *
 * This file demonstrates both polling-based and event-based input handling.
 * You can delete this file after reviewing the examples.
 */

#include "input.hpp"
#include "core/application.hpp"

namespace Platform {

class InputExampleApp : public Core::Application {
   public:
    bool initialize(Window* window) override {
        m_Window = window;

        // Create input system
        m_Input = CreateInput(*window);

        // Option 1: Event-based approach with callbacks
        setupEventCallbacks();

        return true;
    }

    void setupEventCallbacks() {
        // Keyboard callback
        m_Input->setKeyCallback([this](const KeyEvent& event) {
            if (event.action == KeyAction::Press) {
                handleKeyPress(event.key, event.modifiers);
            }
        });

        // Mouse button callback
        m_Input->setMouseButtonCallback([this](const MouseButtonEvent& event) {
            if (event.action == MouseAction::Press) {
                // Handle mouse click at (event.xPos, event.yPos)
            }
        });

        // Mouse move callback (useful for camera control)
        m_Input->setMouseMoveCallback([this](const MouseMoveEvent& event) {
            // event.xDelta and event.yDelta contain movement delta
            updateCameraFromMouse(event.xDelta, event.yDelta);
        });

        // Mouse scroll callback
        m_Input->setMouseScrollCallback([this](const MouseScrollEvent& event) {
            // Handle scroll: event.yOffset is typically used for zoom
            m_CameraZoom += static_cast<float>(event.yOffset) * 0.1f;
        });
    }

    void update(float deltaTime) override {
        // Option 2: Polling-based approach (useful for continuous input like movement)
        handleMovementInput(deltaTime);

        // Option 3: Frame events buffer approach
        processFrameEvents();

        // Clear events after processing
        m_Input->clearFrameEvents();
    }

    void handleMovementInput(float deltaTime) {
        float moveSpeed = 5.0f * deltaTime;

        // WASD movement
        if (m_Input->isKeyPressed(KeyCode::W)) {
            m_CameraPosition.z -= moveSpeed;
        }
        if (m_Input->isKeyPressed(KeyCode::S)) {
            m_CameraPosition.z += moveSpeed;
        }
        if (m_Input->isKeyPressed(KeyCode::A)) {
            m_CameraPosition.x -= moveSpeed;
        }
        if (m_Input->isKeyPressed(KeyCode::D)) {
            m_CameraPosition.x += moveSpeed;
        }

        // Sprint with Shift
        if (m_Input->isKeyPressed(KeyCode::LeftShift)) {
            moveSpeed *= 2.0f;
        }

        // Mouse buttons
        if (m_Input->isMouseButtonPressed(MouseButton::Right)) {
            // Enable camera rotation mode
            m_Input->setCursorMode(false, true);  // Hide and lock cursor
        } else {
            m_Input->setCursorMode(true, false);  // Show cursor
        }
    }

    void processFrameEvents() {
        // Get all events from this frame
        auto events = m_Input->getFrameEvents();

        for (const auto& event : events) {
            switch (event.type) {
                case InputEventType::KeyPress:
                    // Handle key press
                    break;
                case InputEventType::MouseMove:
                    // Handle mouse move
                    break;
                // ... handle other event types
                default:
                    break;
            }
        }
    }

    void handleKeyPress(KeyCode key, ModifierKey mods) {
        // Escape to quit
        if (key == KeyCode::Escape) {
            requestClose();
        }

        // Ctrl+F for fullscreen toggle
        if (key == KeyCode::F && hasModifier(mods, ModifierKey::Control)) {
            toggleFullscreen();
        }

        // Number keys for different actions
        if (key >= KeyCode::Key1 && key <= KeyCode::Key9) {
            int slotNumber = static_cast<int>(key) - static_cast<int>(KeyCode::Key1);
            selectWeaponSlot(slotNumber);
        }
    }

    void updateCameraFromMouse(double dx, double dy) {
        const float sensitivity = 0.1f;
        m_CameraYaw += static_cast<float>(dx) * sensitivity;
        m_CameraPitch += static_cast<float>(dy) * sensitivity;

        // Clamp pitch to avoid gimbal lock
        if (m_CameraPitch > 89.0f)
            m_CameraPitch = 89.0f;
        if (m_CameraPitch < -89.0f)
            m_CameraPitch = -89.0f;
    }

    void render() override {
        // Rendering code...
    }

    void cleanup() override { m_Input.reset(); }

   private:
    std::unique_ptr<Input> m_Input;

    // Camera state (example)
    struct {
        float x, y, z;
    } m_CameraPosition{0, 0, 0};
    float m_CameraYaw{0.0f};
    float m_CameraPitch{0.0f};
    float m_CameraZoom{1.0f};

    void toggleFullscreen() { /* Implementation */ }
    void selectWeaponSlot(int slot) { (void)slot; /* Implementation */ }
};

}  // namespace Platform
