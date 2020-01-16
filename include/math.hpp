#ifndef TITAN_TERRAINS_MATH_HPP_
#define TITAN_TERRAINS_MATH_HPP_

#include <random>

namespace titan::math {
    constexpr float pi = 3.1415926535;

    struct vec2 {
        float x = 0, y = 0;
    };

    template <typename RandEngine>
    static vec2 random_unit_vec2(RandEngine& engine) {
        float angle = ::titan::math::random(0, 2 * pi, engine);
        float x = std::cos(angle);
        float y = std::sin(angle);
        return {x, y};
    }

    inline float dot(vec2 const v1, vec2 const v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }

    inline float magnitude(vec2 const& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    inline vec2 normalize(vec2 const& v) {
        float mag = magnitude(v);
        return {v.x / mag, v.y / mag};
    }

    struct vec3 {
        float x = 0, y = 0, z = 0;

        friend vec3 operator+(vec3 const& a, vec3 const& b) {
            return vec3{a.x + b.x, a.y + b.y, a.z + b.z};
        }
    };

    inline float magnitude(vec3 const& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    inline vec3 normalize(vec3 const& v) {
        float mag = magnitude(v);
        return {v.x / mag, v.y / mag, v.z / mag};
    }

    inline vec3 cross(vec3 const& a, vec3 const& b) {
        return vec3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    }

    template <typename RandEngine>
    float random(float min, float max, RandEngine& engine) {
        std::uniform_real_distribution<float> distr(min, max);
        return distr(engine);
    }

    inline float lerp(float a0, float a1, float w) {
        return (1.0f - w) * a0 + w * a1;
    }

    inline float map_value(float value, float start1, float stop1, float start2, float stop2) {
        return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
    }

    inline size_t index_2d(size_t const x, size_t const y, size_t const w) {
        return y * w + x;
    }
} // namespace titan::math

#endif