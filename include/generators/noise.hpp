#ifndef TITAN_TERRAINS_NOISE_HPP_
#define TITAN_TERRAINS_NOISE_HPP_

#include "math.hpp"

#include <vector>
#include <random>

namespace titan {

class PerlinNoise {
public:
    PerlinNoise(size_t seed);

    std::vector<unsigned char> get_buffer(size_t w, size_t h, size_t octaves = 1);
    void get_buffer(unsigned char* buffer, size_t w, size_t h, size_t octaves = 1);

    float value(float x, float y) const;

    struct gradient_grid {
        math::vec2 size;
        std::vector<math::vec2> gradients;
        std::mt19937 random_engine;

        gradient_grid() = default;
        gradient_grid(size_t w, size_t h, std::mt19937& engine);
        gradient_grid(gradient_grid const&) = default;
        gradient_grid(gradient_grid&&) = default;

        gradient_grid& operator=(gradient_grid const&) = default;
        gradient_grid& operator=(gradient_grid&&) = default;

        math::vec2& at(size_t x, size_t y);
        math::vec2 const& at(size_t x, size_t y) const;
    };

    // regenerates gradient vectors with old scale
    void regenerate_gradients();
    // regenerates gradient vectors with a new scale
    void regenerate_gradients(size_t new_scale);

private:
    gradient_grid gradients;
    size_t scale;
    size_t seed;
    std::mt19937 random_engine;
};


}

#endif