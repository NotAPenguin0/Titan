#version 450 core

in vec2 TexCoords;
in vec3 Normal;

out vec4 FragColor;

void main() {   
    float slope = 1 - dot(vec3(0, 1, 0), Normal);
    if (slope < 0.1) {
        FragColor = vec4(0.486, 0.988, 0, 1);
    } else if (slope < 0.7) {
        FragColor = mix(vec4(0.486, 0.988, 0, 1), vec4(0.584, 0.58, 0.545, 1), (0.7 - slope) / 0.7);
        FragColor = vec4(0.584, 0.58, 0.545, 1);
    } else {
        FragColor = vec4(0.584, 0.58, 0.545, 1);
    }

//    FragColor = vec4(slope, slope, slope, 1);
    FragColor = vec4(Normal, 1);
}