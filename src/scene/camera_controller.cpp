#include "camera_controller.hpp"

#include "camera.hpp"
#include "platform/input/input.hpp"

namespace Scene {

CameraController::CameraController(Camera& camera) : m_Camera(camera) {}

void CameraController::update(const Platform::Input& input, float deltaTime) {
    float speed = m_MoveSpeed * deltaTime;

    if (input.isKeyPressed(Platform::KeyCode::W)) {
        m_Camera.moveForward(speed);
    }
    if (input.isKeyPressed(Platform::KeyCode::S)) {
        m_Camera.moveForward(-speed);
    }
    if (input.isKeyPressed(Platform::KeyCode::A)) {
        m_Camera.moveRight(-speed);
    }
    if (input.isKeyPressed(Platform::KeyCode::D)) {
        m_Camera.moveRight(speed);
    }
    if (input.isKeyPressed(Platform::KeyCode::Space)) {
        m_Camera.moveUp(speed);
    }
    if (input.isKeyPressed(Platform::KeyCode::LeftControl)) {
        m_Camera.moveUp(-speed);
    }

    if (input.isKeyPressed(Platform::KeyCode::LeftShift)) {
        float sprintSpeed = m_MoveSpeed * 2.0f * deltaTime;
        if (input.isKeyPressed(Platform::KeyCode::W)) {
            m_Camera.moveForward(sprintSpeed - speed);
        }
    }
}

void CameraController::process_mouse_move_event(const Platform::MouseMoveEvent& event) {
    if (event.xDelta != 0.0 || event.yDelta != 0.0) {
        m_Camera.rotate(static_cast<float>(event.xDelta) * m_MouseSensitivity,
                        static_cast<float>(-event.yDelta) * m_MouseSensitivity);
    }
}

}  // namespace Scene
