#version 450 core

layout (location = 0) in vec2 iPos;
layout (location = 1) in vec2 iTexCoords;
layout (location = 2) in vec3 iNormal;

layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;
layout(location = 3) uniform sampler2D height_map;
layout(location = 4) uniform float height_scale;


out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

out float Height;

void main() {   
    Normal = iNormal;
    TexCoords = iTexCoords;
    float height = texture(height_map, TexCoords).x;
    Height = height;
    FragPos = vec3(model * vec4(iPos.xy, height, 1));
    gl_Position = projection * view * model * vec4(iPos.xy, (1 - height) * height_scale, 1.0);
}