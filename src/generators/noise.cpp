#include "generators/noise.hpp"

#include <cmath>

#include <stdexcept>

namespace titan {

    using namespace math;

    PerlinNoise::gradient_grid::gradient_grid(size_t w, size_t h, std::mt19937& engine) {
        ++w;
        ++h;
        size.x = w;
        size.y = h;
        gradients.resize(w * h);
        for (int i = 0; i < w * h; ++i) {
            gradients[i] = random_unit_vec2(engine);
        }
    }

    vec2& PerlinNoise::gradient_grid::at(size_t x, size_t y) {
        return gradients[y * (size_t)size.x + x];
    }

    vec2 const& PerlinNoise::gradient_grid::at(size_t x, size_t y) const {
        return gradients[y * (size_t)size.x + x];
    }

    PerlinNoise::PerlinNoise(size_t seed) : seed(seed), random_engine(seed) {}

    std::vector<unsigned char> PerlinNoise::get_buffer(size_t size, size_t octaves) {
        std::vector<unsigned char> buffer(size * size, 0);
        get_buffer(buffer.data(), size, octaves);
        return buffer;
    }

    using u8 = unsigned char;
    using i32 = int;
    using u32 = unsigned int;
    using i64 = long long;
    using u64 = unsigned long long;
    using f32 = float;

    struct Gradient_Grid {
        vec2 gradients[24];
        u8 gradients_size;
        u8 perm_table[128];
        u8 perm_table_size;

        vec2 at(u64 const x, u64 const y) const {
            u8 const index = (y % perm_table_size + x) % perm_table_size;
            return gradients[perm_table[index] % gradients_size];
        }
    };

    static Gradient_Grid create_gradient_grid(u64 const size, std::mt19937& random_engine) {
        Gradient_Grid grid;
        grid.gradients_size = 24;
        grid.perm_table_size = 128;

        for (int i = 0; i < grid.gradients_size; ++i) {
            grid.gradients[i] = random_unit_vec2(random_engine);
        }

        std::uniform_int_distribution<u32> d(0, 255);
        for (int i = 0; i < grid.perm_table_size; ++i) {
            grid.perm_table[i] = d(random_engine);
        }

        return grid;
    }

    static void destroy_gradient_grid(Gradient_Grid grid) {}

    static float perlin_noise(float const x, float const y, vec2 const g00, vec2 const g10, vec2 const g01, vec2 const g11) {
        i64 const x0 = (i64)x;
        i64 const y0 = (i64)y;

        float const x_fractional = x - x0;
        float const y_fractional = y - y0;

        float const fac00 = dot(g00, {x_fractional, y_fractional});
        float const fac10 = dot(g10, {x_fractional - 1.0f, y_fractional});
        float const fac01 = dot(g01, {x_fractional, y_fractional - 1.0f});
        float const fac11 = dot(g11, {x_fractional - 1.0f, y_fractional - 1.0f});

        float const x_lerp_factor = x_fractional * x_fractional * x_fractional * (x_fractional * (x_fractional * 6 - 15) + 10);
        float const y_lerp_factor = y_fractional * y_fractional * y_fractional * (y_fractional * (y_fractional * 6 - 15) + 10);

        float const lerped_x0 = lerp(fac00, fac10, x_lerp_factor);
        float const lerped_x1 = lerp(fac01, fac11, x_lerp_factor);
        return 1.4142135f * lerp(lerped_x0, lerped_x1, y_lerp_factor);
    }

    // size is a power of 2.
    static void generate_noise(unsigned char* const buffer, u32 const size, u32 const octaves, std::mt19937& random_engine) {
        f32 amplitude = 1.0f;
        f32 const persistence = 0.5f;
        f32 const size_f32 = size;
        u64 const size_4aligned = size & (~0x3);
        Gradient_Grid const grid = create_gradient_grid(1 << (octaves - 1), random_engine);

        for (u32 octave = 0; octave < octaves; ++octave) {
            amplitude *= persistence;
            u64 const noise_scale = 1 << octave;
            f32 const noise_scale_f32 = noise_scale;
            f32 const increment = 1.0f / size_f32 * noise_scale_f32;
            u64 const resample_period = size / noise_scale;
            if (resample_period >= 4) {
                for (u64 y = 0; y < size; ++y) {
                    f32 const y_coord = (f32)y / size_f32 * noise_scale_f32;
                    u64 const sample_offset_y = y_coord;
                    for (u64 x = 0, sample_offset_x = 0; sample_offset_x < noise_scale; ++sample_offset_x) {
                        vec2 const g00 = grid.at(sample_offset_x, sample_offset_y);
                        vec2 const g10 = grid.at(sample_offset_x + 1, sample_offset_y);
                        vec2 const g01 = grid.at(sample_offset_x, sample_offset_y + 1);
                        vec2 const g11 = grid.at(sample_offset_x + 1, sample_offset_y + 1);
                        for (u64 i = 0; i < resample_period; i += 4, x += 4) {
                            f32 const x_coord = (f32)x / size_f32 * noise_scale_f32;
                            f32 const val0 = perlin_noise(x_coord, y_coord, g00, g10, g01, g11);
                            f32 const val1 = perlin_noise(x_coord + 1 * increment, y_coord, g00, g10, g01, g11);
                            f32 const val2 = perlin_noise(x_coord + 2 * increment, y_coord, g00, g10, g01, g11);
                            f32 const val3 = perlin_noise(x_coord + 3 * increment, y_coord, g00, g10, g01, g11);
                            buffer[y * size + x] += 255.0f * amplitude * (0.5f + 0.5f * val0);
                            buffer[y * size + x + 1] += 255.0f * amplitude * (0.5f + 0.5f * val1);
                            buffer[y * size + x + 2] += 255.0f * amplitude * (0.5f + 0.5f * val2);
                            buffer[y * size + x + 3] += 255.0f * amplitude * (0.5f + 0.5f * val3);
                        }
                    }
                }
            } else {
                for (u64 y = 0; y < size; ++y) {
                    f32 const y_coord = (f32)y / size_f32 * noise_scale_f32;
                    u64 const sample_offset_y = y_coord;
                    for (u64 x = 0; x < size; ++x) {
                        f32 const x_coord = (f32)x / size_f32 * noise_scale_f32;
                        u64 const sample_offset_x = x_coord;
                        vec2 const g00 = grid.at(sample_offset_x, sample_offset_y);
                        vec2 const g10 = grid.at(sample_offset_x + 1, sample_offset_y);
                        vec2 const g01 = grid.at(sample_offset_x, sample_offset_y + 1);
                        vec2 const g11 = grid.at(sample_offset_x + 1, sample_offset_y + 1);
                        f32 const val = perlin_noise(x_coord, y_coord, g00, g10, g01, g11);
                        buffer[y * size + x] += 255.0f * amplitude * (0.5f + 0.5f * val);
                    }
                }
            }
        }
        destroy_gradient_grid(grid);
    }

    void PerlinNoise::get_buffer(unsigned char* buffer, size_t size, size_t octaves) {
        generate_noise(buffer, size, octaves, random_engine);
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
