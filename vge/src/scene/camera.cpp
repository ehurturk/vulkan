#include "scene/camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Scene {

Camera::Camera(const glm::vec3& position) : m_Position(position) {
    updateCameraVectors();
}

void Camera::moveForward(float amount) {
    m_Position += m_Front * amount;
}

void Camera::moveRight(float amount) {
    m_Position += m_Right * amount;
}

void Camera::moveUp(float amount) {
    m_Position += m_WorldUp * amount;
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    m_Yaw = glm::mod(m_Yaw + yawDelta, 360.0f);
    m_Pitch += pitchDelta;

    if (m_Pitch > 89.0f) {
        m_Pitch = 89.0f;
    }
    if (m_Pitch < -89.0f) {
        m_Pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front{};
    front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    front.y = glm::sin(glm::radians(m_Pitch));
    front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

glm::mat4 Camera::get_view_matrix() const {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::get_projection_matrix(int screenWidth, int screenHeight) const {
    return glm::perspective(glm::radians(m_Fov),
                            static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
                            0.1f, 100.0f);
}

glm::mat4 Camera::get_view_projection_matrix(int screenWidth, int screenHeight) const {
    return get_projection_matrix(screenWidth, screenHeight) * get_view_matrix();
}

}  // namespace Scene
