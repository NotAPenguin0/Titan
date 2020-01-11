#version 450 core

layout(location = 3) uniform sampler2D tex;

out vec4 FragColor;

in vec2 TexCoords;

void main() {   
    FragColor = vec4(1, 0, 0, 1.0);
}