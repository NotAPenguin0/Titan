#include <iostream>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <cmath>

int main() {
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

    while(!glfwWindowShouldClose(win)) {

        float r = std::sin(glfwGetTime());
        float g = std::sin(glfwGetTime() + 5);
        float b = std::sin(glfwGetTime() + 10);

        glClearColor(r, g, b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}