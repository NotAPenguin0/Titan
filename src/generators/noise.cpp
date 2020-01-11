#include "generators/noise.hpp"

#include <cmath>

namespace titan::generators {

using namespace math;

PerlinNoise::gradient_grid::gradient_grid(size_t w, size_t h) {
    ++w;
    ++h;
    size.x = w;
    size.y = h;
    gradients.resize(w * h);
    for (int i = 0; i < w * h; ++i) {
        gradients[i] = normalize(vec2::random());
    }
}

vec2& PerlinNoise::gradient_grid::at(size_t x, size_t y) {
    return gradients[y * (size_t)size.x + x];
}

vec2 const& PerlinNoise::gradient_grid::at(size_t x, size_t y) const {
    return gradients[y * (size_t)size.x + x];
}

static float perlin_lerp(float a0, float a1, float x) {
    float w = 6 * std::pow(x, 5) - 15 * std::pow(x, 4) + 10 * std::pow(x, 3);
    return lerp(a0, a1, w);
}

// x and y are the point's position, ix and iy are the cell coordinates
static float dot_grid_gradient(int ix, int iy, float x, float y, PerlinNoise::gradient_grid const& gradients) {
    // distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    vec2 const& gradient = gradients.at(ix, iy);
    return (dx * gradient.x + dy * gradient.y);
}

PerlinNoise::PerlinNoise(size_t scale /* = 1*/) : gradients(scale, scale), scale(scale) {}

std::vector<unsigned char> PerlinNoise::get_buffer(size_t w, size_t h, size_t octaves) {
    std::vector<unsigned char> buffer(w * h, 0);
    get_buffer(buffer.data(), w, h, octaves);
    return buffer;
}

void PerlinNoise::get_buffer(unsigned char* buffer, size_t w, size_t h, size_t octaves) {
    static float perlin_2d_min = -std::sqrt(0.5f);
    static float perlin_2d_max = std::sqrt(0.5f);
    
    float amplitude = 1.0f;
    float persistence = 0.5f;

    for (size_t octave = 0; octave < octaves; ++octave) {
        regenerate_gradients(std::pow(2, octave));
        amplitude *= persistence;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                int i = y * w + x;
                float val = value((float)x / w * scale, (float)y / h * scale);
                val = amplitude * map_value(val, perlin_2d_min, perlin_2d_max, 0.0f, 1.0f);
                // Map value to range of 0-255
                val *= 255;
                buffer[i] += val;
            }
        }
    }
}

float PerlinNoise::value(float x, float y) const {
    // get grid cell coordinates
    int x0 = (int)x;
    int x1 = x0 + 1;
    int y0 = (int)y;
    int y1 = y0 + 1;

    // interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // interpolate between grid point gradients
    float n0, n1, ix0, ix1, value;
    n0 = dot_grid_gradient(x0, y0, x, y, gradients);
    n1 = dot_grid_gradient(x1, y0, x, y, gradients);
    ix0 = perlin_lerp(n0, n1, sx);

    n0 = dot_grid_gradient(x0, y1, x, y, gradients);
    n1 = dot_grid_gradient(x1, y1, x, y, gradients);
    ix1 = perlin_lerp(n0, n1, sx);

    value = perlin_lerp(ix0, ix1, sy);
    return value;
}

void PerlinNoise::regenerate_gradients() {
    regenerate_gradients(scale);
}

void PerlinNoise::regenerate_gradients(size_t new_scale) {
    gradients = gradient_grid(new_scale, new_scale);
    scale = new_scale;
}

}