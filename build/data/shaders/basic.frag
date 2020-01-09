#version 450 core

layout(location = 0) uniform vec3 color;

layout(location = 4) uniform sampler2D tex;

out vec4 FragColor;

in vec2 TexCoords;

void main() {   
    FragColor = vec4(color * texture(tex, TexCoords).rgb, 1.0);
}