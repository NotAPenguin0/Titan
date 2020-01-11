#ifndef TITAN_TERRAINS_MATH_HPP_
#define TITAN_TERRAINS_MATH_HPP_

namespace titan::math {

constexpr float pi = 3.1415926535;

struct vec2 {
    float x = 0, y = 0;

    static vec2 random();
};

float magnitude(vec2 const& v);
vec2 normalize(vec2 const& v);


float random(float min, float max);

float lerp(float a0, float a1, float w);

float map_value(float value, float start1, float stop1, float start2, float stop2);


}

#endif