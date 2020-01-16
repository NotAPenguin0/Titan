#include "example_app.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>

#include "cinematic_camera.hpp"
#include "renderer/util.hpp"

#include "generators/heightmap_terrain.hpp"

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
    titan::renderer::set_wireframe(false);

    // Load shaders
    unsigned int shader = titan::renderer::load_shader(
        "data/shaders/grid.vert",
        "data/shaders/basic.frag");

    // Create transformation matrices

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 500.0f);
    glm::mat4 model = glm::mat4(1.0);

    model = glm::translate(model, glm::vec3(0, 0, 0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));

    // Create terrain

    float const grid_size = 25.0f;

    titan::HeightmapTerrainInfo info;
    info.width = grid_size;
    info.length = grid_size;
    info.height_scale = 25.0f;
    info.resolution = 10 * grid_size;
    // info.noise_seed = std::random_device()();
    info.noise_seed = 1645;
    info.noise_size = 4096;
    info.noise_layers = 8;

    using namespace std::chrono;

    milliseconds start_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

    titan::HeightmapTerrain terrain = titan::create_heightmap_terrain(info);

    std::chrono::milliseconds end_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

    std::cout << "Generated terrain in " << (end_time - start_time).count() << " ms" << std::endl;

    unsigned int noise_tex = titan::renderer::texture_from_buffer(terrain.height_map.data(), info.noise_size, info.noise_size);

    unsigned int vao;
    unsigned int vbo;
    unsigned int ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, terrain.mesh.vertices.size() * sizeof(float),
                 terrain.mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrain.mesh.indices.size() * sizeof(unsigned int),
                 terrain.mesh.indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao);

    // Positions
    glEnableVertexAttribArray(0);
    glVertexAttribFormat(0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(0, 0);
    glBindVertexBuffer(0, vbo, 0, terrain.mesh.vertex_size * sizeof(float));

    // TexCoords
    glEnableVertexAttribArray(1);
    glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
    glVertexAttribBinding(1, 1);
    glBindVertexBuffer(1, vbo, 0, terrain.mesh.vertex_size * sizeof(float));

    // Normals
    glEnableVertexAttribArray(2);
    glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float));
    glVertexAttribBinding(2, 2);
    glBindVertexBuffer(2, vbo, 0, terrain.mesh.vertex_size * sizeof(float));

    // Index buffer
    glVertexArrayElementBuffer(vao, ebo);

    // Create camera

    auto camera = titan::create_cinematic_camera<titan::OrbitCamera>(glm::vec3(grid_size / 2.0f, -grid_size / 3.0f, grid_size / 2.0f));

    camera.distance_to_target = glm::vec3(grid_size / 2.0f + 25, 10.0F, grid_size / 2.0f + 25);
    camera.rotation_speed = 0.1f;

    float cam_climb_speed = 0.0f;

    while (!glfwWindowShouldClose(win)) {
        float frame_time = glfwGetTime();
        d_time = frame_time - last_frame_time;
        last_frame_time = frame_time;
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        titan::update_cinematic_camera(camera, delta_time());

        glm::mat4 view = titan::get_view_matrix(camera, glm::vec3(0, 1, 0));

        glUseProgram(shader);

        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);
        glUniform1i(3, 0);

        glUniform1f(4, terrain.height_scale);

        glDrawElements(GL_TRIANGLES, terrain.mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}

float Application::delta_time() const {
    return d_time;
}