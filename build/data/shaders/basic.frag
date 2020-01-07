#version 450 core

layout(location = 0) uniform vec3 color;

out vec4 FragColor;

void main() {   
    FragColor = vec4(color, 1.0);
}