#version 450 core

layout (location = 0) in vec3 iPos;

layout(location = 1) uniform mat4 model;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(iPos, 1.0);
}