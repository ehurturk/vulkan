#pragma once

namespace Platform {
class Input;
struct MouseMoveEvent;
}  // namespace Platform

namespace Scene {

class Camera;

class CameraController {
   public:
    explicit CameraController(Camera& camera);
    ~CameraController() = default;

    CameraController(const CameraController&) = delete;
    CameraController& operator=(const CameraController&) = delete;
    CameraController(CameraController&&) = delete;
    CameraController& operator=(CameraController&&) = delete;

    void update(const Platform::Input& input, float deltaTime);

    void process_mouse_move_event(const Platform::MouseMoveEvent& event);

    void setMoveSpeed(float speed) { m_MoveSpeed = speed; }
    void setMouseSensitivity(float sensitivity) { m_MouseSensitivity = sensitivity; }

   private:
    Camera& m_Camera;

    float m_MoveSpeed{5.0f};
    float m_MouseSensitivity{0.1f};
};

}  // namespace Scene
