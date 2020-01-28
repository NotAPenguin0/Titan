#include "camera.hpp"
#include "input.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace titan {

Camera::Camera(glm::vec3 position, glm::vec3 rotation) : pos(position), rotation(rotation) {
    update_vectors();
}

void Camera::update(float dt) {
    do_freelook(dt);
    do_movement(dt);
    update_vectors();
}

glm::mat4 Camera::get_view_matrix() {
    return glm::lookAt(pos, pos + front, up);
}

glm::vec3 Camera::get_position() const {
    return pos;
}

glm::vec3 Camera::get_forward() const {
    return front;
}

void Camera::do_freelook(float dt) {
    auto mouse = RawInput::get_mouse();
    
    float xoffset = mouse.xoffset * mouse_sensitivity * dt;
    float yoffset = mouse.yoffset * mouse_sensitivity * dt;

    rotation.y += xoffset;
    rotation.x += yoffset;

    if(rotation.x > 89.0f) {
        rotation.x =  89.0f;
    } else if(rotation.x < -89.0f) {
        rotation.x = -89.0f;
    }
}

void Camera::do_movement(float dt) {
    float hor = Input::get_axis_raw("Horizontal");
    float ver = Input::get_axis_raw("Vertical");

    glm::vec3 right = glm::normalize(glm::cross(front, up));

    pos += (front * ver * move_speed * dt);
    pos += (right * hor * move_speed * dt);
}

void Camera::update_vectors() {
    float pitch = rotation.x;
    float yaw = rotation.y;
    front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front.y = std::sin(glm::radians(pitch));
    front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

    // calculate up vector for camera
    glm::vec3 direction = glm::normalize(front);
    glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), direction));
    up = glm::normalize(glm::cross(direction, right));
}


}