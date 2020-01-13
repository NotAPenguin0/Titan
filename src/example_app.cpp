#include "example_app.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdexcept>

#include "renderer/util.hpp"
#include "cinematic_camera.hpp"

#include "generators/grid_mesh.hpp"
#include "generators/noise.hpp"

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
    titan::renderer::set_wireframe(true);

    unsigned int shader = titan::renderer::load_shader(
                "data/shaders/grid.vert", 
                "data/shaders/basic.frag");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 500.0f);
    glm::mat4 model = glm::mat4(1.0);

    model = glm::translate(model, glm::vec3(0, 0, 0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));

    float const grid_size = 5.0f;
    titan::GridMesh mesh = titan::generate_grid_mesh(grid_size, grid_size, 10 * grid_size);

    auto camera = titan::create_cinematic_camera<titan::OrbitCamera>(glm::vec3(grid_size / 2, 0, grid_size / 2));

    float camera_height = 3;

    camera.rotation_speed = 0.3f;
    camera.distance_to_target = glm::vec3(grid_size + 5, camera_height, grid_size + 5);

    float cam_climb_speed = 0.0f;


    titan::PerlinNoise noise(1);
    size_t const noise_size = 256;
    auto buf = noise.get_buffer(noise_size, noise_size, 8);
    unsigned int noise_tex = titan::renderer::texture_from_buffer(buf.data(), noise_size, noise_size);

    unsigned int vao;
    unsigned int vbo; 
    unsigned int ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);
    glBindVertexBuffer(0, vbo, 0, 4 * sizeof(float));

    glEnableVertexAttribArray(1);
    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
    glVertexAttribBinding(1, 1);
    glBindVertexBuffer(1, vbo, 0, 4 * sizeof(float));
    glVertexArrayElementBuffer(vao, ebo);

    while(!glfwWindowShouldClose(win)) {
        float frame_time = glfwGetTime();
        d_time = frame_time - last_frame_time;
        last_frame_time = frame_time;

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera_height += cam_climb_speed * delta_time();
        camera.distance_to_target.y = camera_height;

        titan::update_cinematic_camera(camera, delta_time());

        glm::mat4 view = titan::get_view_matrix(camera, glm::vec3(0, 1, 0));

        glUseProgram(shader);

        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);
        glUniform1i(3, 0);

        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}

float Application::delta_time() const {
    return d_time;
}