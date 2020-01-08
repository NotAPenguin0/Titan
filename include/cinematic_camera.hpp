#ifndef TITAN_CINEMATIC_CAMERA_HPP_
#define TITAN_CINEMATIC_CAMERA_HPP_

#include <glm/glm.hpp>

namespace titan {

struct OrbitCamera {
    glm::vec3 target;

    float rotation_speed = 1.0f;
    glm::vec3 distance_to_target = glm::vec3(1, 0, 1);
private:
    glm::vec3 position;

    friend void update_cinematic_camera(OrbitCamera& camera, float delta_time);
    friend glm::mat4 get_view_matrix(OrbitCamera& camera, glm::vec3 world_up);
};

template<typename T>
// T requires CinematicCamera
T create_cinematic_camera(glm::vec3 target = glm::vec3(0, 0, 0)) {
    T camera;

    camera.target = target;

    return camera;
}

void update_cinematic_camera(OrbitCamera& camera, float delta_time);

glm::mat4 get_view_matrix(OrbitCamera& camera, glm::vec3 world_up);

}

#endif