#ifndef TITAN_TERRAINS_NOISE_HPP_
#define TITAN_TERRAINS_NOISE_HPP_

#include "math.hpp"

#include <vector>

namespace titan {

namespace generators {

class PerlinNoise {
public:
    PerlinNoise(size_t scale = 1);

    std::vector<unsigned char> get_buffer(size_t w, size_t h) const;
    void get_buffer(unsigned char* buffer, size_t w, size_t h) const;

    float value(float x, float y) const;

    struct gradient_grid {
        math::vec2 size;
        std::vector<math::vec2> gradients;

        gradient_grid(size_t w, size_t h);
        gradient_grid(gradient_grid const&) = default;
        gradient_grid(gradient_grid&&) = default;

        gradient_grid& operator=(gradient_grid const&) = default;
        gradient_grid& operator=(gradient_grid&&) = default;

        math::vec2& at(size_t x, size_t y);
        math::vec2 const& at(size_t x, size_t y) const;
    };

private:
    gradient_grid gradients;
    size_t scale;
};

}

}

#endif