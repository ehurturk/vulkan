#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace Scene {

class Camera {
   public:
    Camera();
    Camera(const glm::vec3& position);

    glm::mat4 get_view_projection_matrix();
    glm::mat4 get_view_matrix();
    glm::mat4 get_projection_matrix();

   private:
    glm::vec3 m_Position;
    glm::vec3 m_Target;

    glm::vec3 m_Front;
    glm::vec3 m_Up;
    [[maybe_unused]] glm::vec3 m_Right;
};

}  // namespace Scene
