#ifndef TITAN_TERRAINS_MATH_HPP_
#define TITAN_TERRAINS_MATH_HPP_

#include <random>

namespace titan::math {

constexpr float pi = 3.1415926535;

struct vec2 {
    float x = 0, y = 0;

    template<typename RandEngine>
    static vec2 random(RandEngine& engine) {
        float angle = ::titan::math::random(0, 2 * pi, engine);
        float x = std::cos(angle);
        float y = std::sin(angle);
        return {x, y};
    }
};

struct vec3 {
    float x = 0, y = 0, z = 0;

    friend vec3 operator+(vec3 const& a, vec3 const& b) {
        return vec3{a.x + b.x, a.y + b.y, a.z + b.z};
    }
};

float magnitude(vec2 const& v);
float magnitude(vec3 const& v);
vec2 normalize(vec2 const& v);
vec3 normalize(vec3 const& v);

vec3 cross(vec3 const& a, vec3 const& b);

template<typename RandEngine>
float random(float min, float max, RandEngine& engine) {
    std::uniform_real_distribution<float> distr(min, max);
    return distr(engine);
}

float lerp(float a0, float a1, float w);

float map_value(float value, float start1, float stop1, float start2, float stop2);

inline size_t index_2d(size_t const x, size_t const y, size_t const w) {
    return y * w + x;
}

}

#endif