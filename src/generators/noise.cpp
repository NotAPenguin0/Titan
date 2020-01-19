#include "generators/noise.hpp"

#include <cmath>

#include <stdexcept>

namespace titan {

using namespace math;

PerlinNoise::PerlinNoise(size_t seed) : seed(seed), random_engine(seed) {}

std::vector<unsigned char> PerlinNoise::get_buffer(size_t size, size_t octaves, float persistence) {
    std::vector<unsigned char> buffer(size * size, 0);
    get_buffer(buffer.data(), size, octaves, persistence);
    return buffer;
}

std::vector<float> PerlinNoise::get_buffer_float(size_t size, size_t octaves, float persistence) {
    std::vector<float> buffer(size * size, 0);
    get_buffer(buffer.data(), size, octaves, persistence);
    return buffer;
}

using u8 = unsigned char;
using i32 = int;
using u32 = unsigned int;
using i64 = long long;
using u64 = unsigned long long;
using f32 = float;

struct GradientGrid {
    vec2 gradients[24];
    u8 gradients_size;
    u8 perm_table[128];
    u8 perm_table_size;

    vec2 at(u64 const x, u64 const y) const {
        u8 const index = (y % perm_table_size + x) % perm_table_size;
        return gradients[perm_table[index] % gradients_size];
    }
};

static GradientGrid create_gradient_grid(u64 const size, std::mt19937& random_engine) {
    GradientGrid grid;
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

static void destroy_gradient_grid(GradientGrid grid) {}

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
static void generate_noise(unsigned char* const buffer, u32 const size, u32 const octaves, f32 const persistence,
                           std::mt19937& random_engine) {
    f32 amplitude = 1.0f;
    f32 const size_f32 = size;
    u64 const size_4aligned = size & (~0x3);
    GradientGrid const grid = create_gradient_grid(1 << (octaves - 1), random_engine);

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

// Same function but for float buffer
static void generate_noise(float* buffer, u32 const size, u32 const octaves, f32 const persistence,
                           std::mt19937& random_engine) {
    f32 amplitude = 1.0f;
    f32 const size_f32 = size;
    u64 const size_4aligned = size & (~0x3);
    GradientGrid const grid = create_gradient_grid(1 << (octaves - 1), random_engine);

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
                        buffer[y * size + x] += amplitude * (0.5f + 0.5f * val0);
                        buffer[y * size + x + 1] += amplitude * (0.5f + 0.5f * val1);
                        buffer[y * size + x + 2] += amplitude * (0.5f + 0.5f * val2);
                        buffer[y * size + x + 3] += amplitude * (0.5f + 0.5f * val3);
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
                    buffer[y * size + x] += amplitude * (0.5f + 0.5f * val);
                }
            }
        }
    }
    destroy_gradient_grid(grid);
}

void PerlinNoise::get_buffer(unsigned char* buffer, size_t size, size_t octaves, float persistence) {
    generate_noise(buffer, size, octaves, persistence, random_engine);
}

void PerlinNoise::get_buffer(float* buffer, size_t size, size_t octaves, float persistence) {
    generate_noise(buffer, size, octaves, persistence, random_engine);
}


} // namespace titan
