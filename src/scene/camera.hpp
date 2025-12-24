#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace Scene {

class Camera {
   public:
    Camera() = default;
    Camera(const glm::vec3& position);

    void moveForward(float amount);
    void moveRight(float amount);
    void moveUp(float amount);
    void rotate(float yawDelta, float pitchDelta);

    glm::mat4 get_view_projection_matrix(int screenWidth, int screenHeight) const;
    glm::mat4 get_view_matrix() const;
    glm::mat4 get_projection_matrix(int screenWidth, int screenHeight) const;

    [[nodiscard]] const glm::vec3& getPosition() const { return m_Position; }
    [[nodiscard]] const glm::vec3& getFront() const { return m_Front; }
    [[nodiscard]] float getFov() const { return m_Fov; }

    void setFov(float fov) { m_Fov = fov; }

   private:
    void updateCameraVectors();

    glm::vec3 m_Position{0.0f, 0.0f, 0.0f};
    glm::vec3 m_Front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_Up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_Right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_WorldUp{0.0f, 1.0f, 0.0f};

    float m_Yaw{-90.0f};
    float m_Pitch{0.0f};
    float m_Fov{45.0f};
};

}  // namespace Scene
