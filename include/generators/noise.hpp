#ifndef TITAN_TERRAINS_NOISE_HPP_
#define TITAN_TERRAINS_NOISE_HPP_

#include "math.hpp"

#include <random>
#include <vector>

namespace titan {

class PerlinNoise {
public:
    PerlinNoise(size_t seed);

    std::vector<unsigned char> get_buffer(size_t size, size_t octaves = 1, float persistence = 0.5f);
    void get_buffer(unsigned char* buffer, size_t size, size_t octaves = 1, float persistence = 0.5f);

private:
    size_t seed;
    std::mt19937 random_engine;
};

} // namespace titan

#endif