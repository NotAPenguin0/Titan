#include "generators/noise.hpp"

#include <cmath>

namespace titan {

    using namespace math;

    PerlinNoise::gradient_grid::gradient_grid(size_t w, size_t h, std::mt19937& engine) {
        ++w;
        ++h;
        size.x = w;
        size.y = h;
        gradients.resize(w * h);
        for (int i = 0; i < w * h; ++i) {
            gradients[i] = normalize(vec2::random(engine));
        }
    }

    vec2& PerlinNoise::gradient_grid::at(size_t x, size_t y) {
        return gradients[y * (size_t)size.x + x];
    }

    vec2 const& PerlinNoise::gradient_grid::at(size_t x, size_t y) const {
        return gradients[y * (size_t)size.x + x];
    }

    PerlinNoise::PerlinNoise(size_t seed) : seed(seed), random_engine(seed) {}

    std::vector<unsigned char> PerlinNoise::get_buffer(size_t w, size_t h, size_t octaves) {
        std::vector<unsigned char> buffer(w * h, 0);
        get_buffer(buffer.data(), w, h, octaves);
        return buffer;
    }

    static PerlinNoise::gradient_grid generate_gradients(size_t const size, std::mt19937& random_engine) {
        return PerlinNoise::gradient_grid(size, size, random_engine);
    }

    static float perlin_noise(float const x, float const y, PerlinNoise::gradient_grid const& gradients) {
        int const x0 = (int)x;
        int const x1 = x0 + 1;
        int const y0 = (int)y;
        int const y1 = y0 + 1;

        float const x_fractional = x - x0;
        float const y_fractional = y - y0;

        vec2 const g00 = gradients.at(x0, y0);
        vec2 const g10 = gradients.at(x0, y0);
        vec2 const g01 = gradients.at(x0, y1);
        vec2 const g11 = gradients.at(x1, y1);

        float const fac00 = dot(g00, {x_fractional, y_fractional});
        float const fac10 = dot(g10, {x_fractional - 1.0f, y_fractional});
        float const fac01 = dot(g01, {x_fractional, y_fractional - 1.0f});
        float const fac11 = dot(g11, {x_fractional - 1.0f, y_fractional - 1.0f});

        float const x_lerp_factor = x_fractional * x_fractional * x_fractional * (x_fractional * (x_fractional * 6 - 15) + 10);
        float const y_lerp_factor = y_fractional * y_fractional * y_fractional * (y_fractional * (y_fractional * 6 - 15) + 10);

        float const lerped_x0 = lerp(fac00, fac10, x_lerp_factor);
        float const lerped_x1 = lerp(fac01, fac11, x_lerp_factor);
        float noise = lerp(lerped_x0, lerped_x1, y_lerp_factor);
        return 1.4142135f * noise;
    }

    void PerlinNoise::get_buffer(unsigned char* buffer, size_t w, size_t h, size_t octaves) {
        float amplitude = 1.0f;
        float const persistence = 0.5f;

        for (size_t octave = 0; octave < octaves; ++octave) {
            regenerate_gradients(1 << octave);
            PerlinNoise::gradient_grid const gradients_loop = generate_gradients(1 << octave, random_engine);
            amplitude *= persistence;
            for (size_t y = 0; y < h; ++y) {
                float const y_coord = (float)y / h * scale;
                for (size_t x = 0; x < w; x += 4) {
                    float const val0 = amplitude * (0.5f + 0.5f * perlin_noise((float)x / w * scale, y_coord, gradients_loop));
                    float const val1 = amplitude * (0.5f + 0.5f * perlin_noise((float)(x + 1) / w * scale, y_coord, gradients_loop));
                    float const val2 = amplitude * (0.5f + 0.5f * perlin_noise((float)(x + 2) / w * scale, y_coord, gradients_loop));
                    float const val3 = amplitude * (0.5f + 0.5f * perlin_noise((float)(x + 3) / w * scale, y_coord, gradients_loop));
                    buffer[y * w + x] += val0 * 255.0f;
                    buffer[y * w + x + 1] += val1 * 255.0f;
                    buffer[y * w + x + 2] += val2 * 255.0f;
                    buffer[y * w + x + 3] += val3 * 255.0f;
                }

                size_t const w4 = w / 4;
                for (size_t x = w4 * 4; x < w; ++x) {
                    float const val = amplitude * (0.5f + 0.5f * perlin_noise((float)x / w * scale, y_coord, gradients_loop));
                    buffer[y * w + x] += val * 255.0f;
                }
            }
        }
    }

    static float perlin_lerp(float a0, float a1, float x) {
        //    float w = 6 * std::pow(x, 5) - 15 * std::pow(x, 4) + 10 * std::pow(x, 3); ==>  SLOW, line below is 10 times faster
        float w = x * x * x * (x * (x * 6 - 15) + 10);
        return lerp(a0, a1, w);
    }

    // x and y are the point's position, ix and iy are the cell coordinates
    static float dot_grid_gradient(int ix, int iy, float x, float y, PerlinNoise::gradient_grid const& gradients) {
        // distance vector
        float dx = x - (float)ix;
        float dy = y - (float)iy;

        vec2 const gradient = gradients.at(ix, iy);
        return (dx * gradient.x + dy * gradient.y);
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
        gradients = gradient_grid(new_scale, new_scale, random_engine);
        scale = new_scale;
    }

} // namespace titan
