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
#include "example_app.hpp"

#include "renderer/util.hpp"

int main() try {
    
    titan::Application app(800, 600);
    app.run();

} catch(std::exception const& e) {
    std::cerr << e.what() << std::endl;
}