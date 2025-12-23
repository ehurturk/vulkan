#include "camera.hpp"

namespace Scene {

Camera::Camera()
    : m_Position{0.0f, 0.0f, 3.0f},
      m_Target{0.0f, 0.0f, 0.0f},
      m_Front{0.0f, 0.0f, -1.0f},
      m_Up{0.0f, 1.0f, 0.0f} {}

Camera::Camera(const glm::vec3& position)
    : m_Position{position},
      m_Target{0.0f, 0.0f, 0.0f},
      m_Front{0.0f, 0.0f, -1.0f},
      m_Up{0.0f, 1.0f, 0.0f} {}

glm::mat4 Camera::get_view_projection_matrix() {
    return get_view_matrix() * get_projection_matrix();
}

glm::mat4 Camera::get_view_matrix() {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::get_projection_matrix() {
    return glm::mat4{};
}

}  // namespace Scene
