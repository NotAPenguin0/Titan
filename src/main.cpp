#include <iostream>

#include <glfw/glfw3.h>

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

    while(!glfwWindowShouldClose(win)) {

        glfwPollEvents();
        glfwSwapBuffers(win);
    }
}