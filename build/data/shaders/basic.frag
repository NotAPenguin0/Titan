#version 450 core

layout(location = 3) uniform sampler2D tex;

out vec4 FragColor;

in vec2 TexCoords;

void main() {   
    FragColor = vec4(texture(tex, TexCoords).rgb, 1.0);
}