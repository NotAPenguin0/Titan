#version 450 core

layout(location = 3) uniform sampler2D tex;

out vec4 FragColor;

in vec2 TexCoords;

void main() {   
    float val = texture(tex, TexCoords).r;
    FragColor = vec4(val, val, val, 1.0);
}