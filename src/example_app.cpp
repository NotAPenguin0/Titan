#include "example_app.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

#include "renderer/util.hpp"
#include "cinematic_camera.hpp"

Application::Application(size_t const width, size_t const height) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    win = glfwCreateWindow(width, height, "Titan playground", nullptr, nullptr);
    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to load glad");
    }
}

Application::~Application() {
    glfwDestroyWindow(win);
    glfwTerminate();
}

void Application::run() {
    unsigned int shader = titan::renderer::load_shader(
                "data/shaders/basic.vert", 
                "data/shaders/basic.frag");

    unsigned int tex = titan::renderer::load_texture("data/textures/pengu.png");

    float const vertices[] = {-1, -1, 0, 0, 1, 0, 1, -1, 0};
    float const tex_coords[] = {0, 0, 0.5, 1, 1, 0};
    unsigned int vbo, vao;
    unsigned int texcoords_buf;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &texcoords_buf);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    glBindVertexBuffer(0, vbo, 0, 3 * sizeof(float));
    glVertexAttribBinding(0, 0);
    glEnableVertexAttribArray(0);

    // Texcoords
    glBindBuffer(GL_ARRAY_BUFFER, texcoords_buf);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), tex_coords, GL_STATIC_DRAW);

    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);
    glBindVertexBuffer(1, texcoords_buf, 0, 2 * sizeof(float));
    glVertexAttribBinding(1, 1);
    glEnableVertexAttribArray(1);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0);

    auto camera = titan::create_cinematic_camera<titan::OrbitCamera>(glm::vec3(0, 0, 0));

    float camera_height = 0;

    camera.rotation_speed = 0.3f;
    camera.distance_to_target = glm::vec3(5, camera_height, 5);

    float cam_climb_speed = 0.0f;

    while(!glfwWindowShouldClose(win)) {
        float frame_time = glfwGetTime();
        d_time = frame_time - last_frame_time;
        last_frame_time = frame_time;

        camera_height += cam_climb_speed * delta_time();
        camera.distance_to_target.y = camera_height;

        float r = std::sin(glfwGetTime());
        float g = std::sin(glfwGetTime() + 5);
        float b = std::sin(glfwGetTime() + 10);

        float rgb[] = {r, g, b};

        titan::update_cinematic_camera(camera, delta_time());

        glm::mat4 view = titan::get_view_matrix(camera, glm::vec3(0, 1, 0));

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniform3fv(0, 1, rgb);

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glUniform1i(4, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}

float Application::delta_time() const {
    return d_time;
}