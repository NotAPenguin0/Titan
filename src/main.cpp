#include <iostream>
#include <exception>

#include "example_app.hpp"

int main() try {
    Application app(1280, 720);
    app.run();

} catch(std::exception const& e) {
    std::cerr << e.what() << std::endl;
}