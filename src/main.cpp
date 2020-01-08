#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <cmath>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "cinematic_camera.hpp"

std::string read_file(const char* path) {
    using namespace std::literals::string_literals;
    std::ifstream f(path);
    if (!f.good()) {
        throw std::runtime_error("Failed to open file: "s + path);
    }
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

unsigned int create_shader_stage(GLenum stage, const char* source) {
    using namespace std::literals::string_literals;
    unsigned int shader = glCreateShader(stage);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infolog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infolog);
        throw std::runtime_error("Failed to compile shader:\n"s + source + "\nReason: "s + infolog);
    }

    return shader;
}

unsigned int create_shader(const char* vertex, const char* fragment) {
    using namespace std::literals::string_literals;
    unsigned int vtx = create_shader_stage(GL_VERTEX_SHADER, vertex);
    unsigned int frag = create_shader_stage(GL_FRAGMENT_SHADER, fragment);

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vtx);
    glAttachShader(prog, frag);

    glLinkProgram(prog);
    int success;
    char infolog[512];
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, nullptr, infolog);
        throw std::runtime_error("Failed to link shader.\nReason: "s + infolog);
        return 0;
    }

    glDeleteShader(vtx);
    glDeleteShader(frag);

    return prog;
}

int main() try {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize glfw" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* win = glfwCreateWindow(800, 600, "Titan playground", nullptr, nullptr);

    glfwMakeContextCurrent(win);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to load glad\n";
        return 1;
    }

    unsigned int shader;
    [&shader](){
        std::string vertex = read_file("data/shaders/basic.vert");
        std::string frag = read_file("data/shaders/basic.frag");
        shader = create_shader(vertex.c_str(), frag.c_str());
    }();

    float vertices[] = {-1, -1, 0, 0, 1, 0, 1, -1, 0};
    unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0);

    auto camera = titan::create_cinematic_camera<titan::OrbitCamera>(glm::vec3(0, 0, 0));

    float camera_height = 0;

    camera.rotation_speed = 0.3f;
    camera.distance_to_target = glm::vec3(3, camera_height, 3);

    float cam_climb_speed = 0.3f;

    float last_frame = 0;

    while(!glfwWindowShouldClose(win)) {
        float frame_time = glfwGetTime();
        float delta_time = frame_time - last_frame;
        last_frame = frame_time;

        camera_height += cam_climb_speed * delta_time;
        camera.distance_to_target.y = camera_height;

        float r = std::sin(glfwGetTime());
        float g = std::sin(glfwGetTime() + 5);
        float b = std::sin(glfwGetTime() + 10);

        float rgb[] = {r, g, b};

        titan::update_cinematic_camera(camera, delta_time);

        glm::mat4 view = titan::get_view_matrix(camera, glm::vec3(0, 1, 0));

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glUniform3fv(0, 1, rgb);

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(projection));

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
} catch(std::exception const& e) {
    std::cerr << e.what() << std::endl;
}