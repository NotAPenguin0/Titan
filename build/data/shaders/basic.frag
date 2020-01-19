#version 450 core

in vec2 TexCoords;
in vec3 Normal;

layout(binding = 1) uniform sampler2D grass;
layout(binding = 2) uniform sampler2D moss;
layout(binding = 3) uniform sampler2D stone;

layout(location = 5) uniform float terrain_scale;

out vec4 FragColor;
in float Height;

vec3 directional_light(vec3 in_color) {
    // hardcoded light in shader :/
    vec3 direction = normalize(-vec3(-0.2, -1, -0.3));
    float light_ambient = 0.33;
    float light_diffuse = 0.93;
    // ambient
    vec3 ambient = light_ambient * in_color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, direction), 0.0);
    vec3 diffuse = light_diffuse * diff * in_color;  
    
    vec3 result = ambient + diffuse;
    return result;
}

void main() {   
    float slope = 1 - dot(vec3(0, 1, 0), Normal);
    
    vec3 grass_color = texture(grass, TexCoords * terrain_scale).rgb;
    vec3 moss_color = texture(moss, TexCoords * terrain_scale).rgb;
    vec3 stone_color = texture(stone, TexCoords * terrain_scale).rgb;

    vec3 color = vec3(0);

    if (slope < 0.3) {
        color = grass_color;
    } else if (slope < 0.35) {
        color = mix(moss_color, grass_color, (0.35 - slope) / 0.05);
    } else if (slope < 0.4) {
        color = moss_color;
    } else if (slope < 0.7) {
        color = mix(stone_color, moss_color, (0.7 - slope) / 0.3);
    } else {
        color = stone_color;
    }

    color = directional_light(color);

    FragColor = vec4(color, 1);
}