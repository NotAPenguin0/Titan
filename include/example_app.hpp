#ifndef TITAN_EXAMPLE_APP_HPP_
#define TITAN_EXAMPLE_APP_HPP_


#include <glad/glad.h>
#include <glfw/glfw3.h>

class Application {
public:
    Application(size_t const width, size_t const height);
    
    ~Application();

    void run();

    float delta_time() const;

private:
    // Windowing
    GLFWwindow* win;

    // Timing
    float d_time = 0;
    float last_frame_time = 0;
};


#endif