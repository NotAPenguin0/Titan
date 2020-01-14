#include "math.hpp"

#include <cmath>
#include <random>

namespace titan::math {
    float magnitude(vec2 const& v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    float magnitude(vec3 const& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    vec2 normalize(vec2 const& v) {
        float mag = magnitude(v);
        return {v.x / mag, v.y / mag};
    }

    vec3 normalize(vec3 const& v) {
        float mag = magnitude(v);
        return {v.x / mag, v.y / mag, v.z / mag};
    }

    vec3 cross(vec3 const& a, vec3 const& b) {
        return vec3{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    }
} // namespace titan::math