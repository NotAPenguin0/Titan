#include "math.hpp"

#include <cmath>
#include <random>


namespace titan::math {


float magnitude(vec2 const& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

vec2 normalize(vec2 const& v) {
    float mag = magnitude(v);
    return {v.x / mag, v.y / mag};
}

float random(float min, float max) {
    static std::mt19937 engine(std::random_device{}());

    std::uniform_real_distribution<float> distr(min, max);
    return distr(engine);
}

vec2 vec2::random() {
    float angle = ::titan::math::random(0, 2 * pi);
    float x = std::cos(angle);
    float y = std::sin(angle);
    return {x, y};
}

float lerp(float a0, float a1, float w) {
    return a0 + w * (a1 - a0);
}

float map_value(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + (stop2 - start2) * ((value - start1) / (stop1 - start1));
}

}