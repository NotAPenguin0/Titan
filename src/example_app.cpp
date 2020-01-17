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
#include "input.hpp"
#include "camera.hpp"

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

    Input::initialize(win);
    InputEventManager::init(win);

    AxisManager::add_axis("Horizontal");
    AxisManager::add_axis("Vertical");

    AxisMapping pos_hor_mapping;
    pos_hor_mapping.key = Key::D;
    pos_hor_mapping.name = "Horizontal";
    
    AxisMapping neg_hor_mapping;
    neg_hor_mapping.key = Key::A;
    neg_hor_mapping.name = "Horizontal";
    neg_hor_mapping.sensitivity = -1;

    AxisMapping pos_ver_mapping;
    pos_ver_mapping.key = Key::W;
    pos_ver_mapping.name = "Vertical";

    AxisMapping neg_ver_mapping;
    neg_ver_mapping.key = Key::S;
    neg_ver_mapping.name = "Vertical";
    neg_ver_mapping.sensitivity = -1;

    AxisManager::add_axis_mapping(pos_hor_mapping);
    AxisManager::add_axis_mapping(neg_hor_mapping);
    AxisManager::add_axis_mapping(pos_ver_mapping);
    AxisManager::add_axis_mapping(neg_ver_mapping);

    Input::set_mouse_capture(true);

    // Load shaders
    unsigned int shader = titan::renderer::load_shader(
        "data/shaders/grid.vert",
        "data/shaders/basic.frag");

    unsigned int grass = titan::renderer::load_texture("data/textures/grass.png");
    unsigned int moss = titan::renderer::load_texture("data/textures/moss.png");
    unsigned int stone = titan::renderer::load_texture("data/textures/stone.png");

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
    info.resolution = 50 * grid_size;
    info.noise_seed = std::random_device()();
    info.noise_size = 2048;
    info.noise_layers = 3;
    info.noise_persistence = 0.2f;

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

    titan::Camera camera(glm::vec3(0, 2, 0));
    camera.mouse_sensitivity = 0.02f;
    camera.move_speed = 0.001f;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (!glfwWindowShouldClose(win)) {
        float frame_time = glfwGetTime();
        d_time = frame_time - last_frame_time;
        last_frame_time = frame_time;

        InputEventManager::process_events(frame_time);

        if (RawInput::get_key(Key::Escape).down) {
            glfwSetWindowShouldClose(win, true);
        }

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.update(frame_time);

        glm::mat4 view = camera.get_view_matrix();

        glUseProgram(shader);

        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1f(5, grid_size);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, noise_tex);
        glUniform1i(3, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grass);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, moss);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, stone);

        glUniform1f(4, terrain.height_scale);

        glDrawElements(GL_TRIANGLES, terrain.mesh.indices.size(), GL_UNSIGNED_INT, nullptr);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}

float Application::delta_time() const {
    return d_time;
}