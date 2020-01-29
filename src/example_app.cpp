#include "example_app.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <random>
#include <stdexcept>

#include "cinematic_camera.hpp"
#include "input.hpp"
#include "camera.hpp"

#include "renderer/terrain_renderer.hpp"
#include "renderer/util.hpp"

#include "generators/heightmap_terrain.hpp"

static void gl_error_callback([[maybe_unused]] GLenum source, 
                       [[maybe_unused]] GLenum type, 
                       [[maybe_unused]] GLuint id,
                       [[maybe_unused]] GLenum severity,
                       [[maybe_unused]] GLsizei length, 
                       [[maybe_unused]] GLchar const* message,
                       [[maybe_unused]] void const* user_data) {
    
    std::string err_type = "Unknown error";
    if (type == GL_DEBUG_TYPE_ERROR) {
        err_type = "Error";
    } else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) {
        err_type = "Deprecated behavior";
    } else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) {
        err_type = "Undefined behavior";
    } else if (type == GL_DEBUG_TYPE_PORTABILITY) {
        err_type = "Non portable functionality";
    } else if (type == GL_DEBUG_TYPE_PERFORMANCE) {
        err_type = "Performance issue";
    } else if (type == GL_DEBUG_TYPE_MARKER) {
        err_type = "Command stream annotation";
    } else if (type == GL_DEBUG_TYPE_OTHER) {
        err_type = "Other error";
    }

    std::string severity_string = "Unknown";
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) severity_string == "Info";
    else if (severity == GL_DEBUG_SEVERITY_LOW) severity_string = "Warning";
    else if (severity == GL_DEBUG_SEVERITY_MEDIUM) severity_string = "Error";
    else if (severity == GL_DEBUG_SEVERITY_HIGH) severity_string = "Critical error";

    std::cerr << "[OpenGL] " << severity_string << ": " << message << std::endl;
}

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

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_error_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, nullptr, GL_FALSE);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

Application::~Application() {
    glfwDestroyWindow(win);
    glfwTerminate();
}

/*
static void update_lod(titan::HeightmapTerrain& terrain, size_t new_lod, unsigned int vbo, unsigned int ebo) {
    auto& mesh = terrain.meshes[new_lod];
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
                 mesh.indices.data(), GL_STATIC_DRAW);
}
*/


void Application::run() {

/////////
// https://discordapp.com/channels/318590007881236480/318783155744145411/668564420435378177
// Rodrigo being the true MPV explaining me how to use persistent maps
////////

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

//    model = glm::translate(model, glm::vec3(0, 0, 0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));

    glm::mat4 inv_model = glm::inverse(model);

    // Create terrain

    float const grid_size = 80.0f;

    titan::HeightmapTerrainInfo info;
    info.width = grid_size;
    info.length = grid_size;
    info.height_scale = 100.0f;
    info.max_lod = 100;
    info.noise_seed = std::random_device()();
    info.noise_size = 4096;
    info.noise_layers = 4;
    info.noise_persistence = 0.5f;

    using namespace std::chrono;

    milliseconds start_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

    titan::HeightmapTerrain terrain = titan::create_heightmap_terrain(info);

    std::chrono::milliseconds end_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    std::cout << "Generated terrain in " << (end_time - start_time).count() << " ms" << std::endl;

    std::cout << "Max LOD: " << info.max_lod << std::endl;
    std::cout << "Total LOD count: " << terrain.max_lod << std::endl;

    size_t const lod = 0;
    size_t cur_lod = terrain.max_lod / 2;

    start_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

    titan::renderer::TerrainRenderInfo render_info = titan::renderer::make_terrain_render_info(terrain, terrain.max_lod / 2);

    end_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
    std::cout << "Data upload finished  in " << (end_time - start_time).count() << " ms" << std::endl;

    // Create camera

    titan::Camera camera(glm::vec3(0, 2, 0));
    camera.mouse_sensitivity = 5.0f;
    camera.move_speed = 5.0f;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Increasing LOD means going to a lower index

    ActionBinding increase_lod;
    increase_lod.key = Key::Up;
    increase_lod.when = KeyAction::Press;
    increase_lod.callback = [&terrain, &cur_lod, &render_info] () {
        if (cur_lod == 0) { return; }
        titan::renderer::higher_lod(render_info, terrain, 0);
        --cur_lod;
    };

    // Decreasing LOD means going to a higher index

    ActionBinding decrease_lod;
    decrease_lod.key = Key::Down;
    decrease_lod.when = KeyAction::Press;
    decrease_lod.callback = [&terrain, &cur_lod, &render_info] () {
        if (cur_lod == terrain.max_lod - 1) { return; }
        titan::renderer::lower_lod(render_info, terrain, 0);
        ++cur_lod;
    };

    ActionBindingManager::add_action(increase_lod);
    ActionBindingManager::add_action(decrease_lod);

    ActionBinding quit;
    quit.key = Key::Escape;
    quit.when = KeyAction::Press;
    quit.callback = [this] () {
        glfwSetWindowShouldClose(win, true);
    };

    ActionBindingManager::add_action(quit);

    for (auto& chunk : render_info.chunks) {
        titan::renderer::await_all_data_upload(chunk);
    }

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

        camera.update(d_time);
        glm::vec3 pos = camera.get_position();
        
        titan::renderer::update_lod_distance(render_info, terrain, model, glm::value_ptr(pos));

        glm::mat4 view = camera.get_view_matrix();

        glUseProgram(shader);

        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform1f(5, grid_size);
//        glUniform3fv(6, 1, glm::value_ptr(camera.get_position()));
//        glUniform3fv(7, 1, glm::value_ptr(camera.get_forward()));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grass);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, moss);

        glActiveTexture(GL_TEXTURE3);

        glBindTexture(GL_TEXTURE_2D, stone);

        glUniform1f(4, terrain.height_scale);

        titan::renderer::render_terrain(render_info);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}

float Application::delta_time() const {
    return d_time;
}