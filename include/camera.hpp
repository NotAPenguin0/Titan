#ifndef TITAN_CAMERA_HPP_
#define TITAN_CAMERA_HPP_

#include <glm/glm.hpp>

namespace titan {

class Camera {
public:
    Camera(glm::vec3 position = glm::vec3(0, 0, 0), glm::vec3 rotation = glm::vec3(-45.0f, 0, 0));

    void update(float dt);
    glm::mat4 get_view_matrix();

    float mouse_sensitivity = 0.05f;
    float move_speed = 0.05f;

private:
    void do_freelook(float dt);
    void do_movement(float dt);
    void update_vectors();

    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 rotation;
    glm::vec3 up;
};

}

#endif