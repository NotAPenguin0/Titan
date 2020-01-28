#version 450 core

in vec2 TexCoords;
in vec3 Normal;
in float Height;
in vec3 FragPos;

layout(binding = 1) uniform sampler2D grass;
layout(binding = 2) uniform sampler2D moss;
layout(binding = 3) uniform sampler2D stone;

layout(location = 5) uniform float terrain_scale;
layout(location = 6) uniform vec3 CamPos;
//layout(location = 7) uniform vec3 CamDir;

out vec4 FragColor;


vec3 directional_light(vec3 in_color) {
    // hardcoded light in shader :/
    vec3 direction = normalize(-vec3(-0.2, -1, -0.3));
    float light_ambient = 0.3;
    float light_diffuse = 10.5;
    // ambient
    vec3 ambient = light_ambient * in_color;
  	
    // diffuse 
    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, direction), 0.0);
    vec3 diffuse = light_diffuse * diff * in_color;  
    
    vec3 result = ambient + diffuse;
    return result;
}

vec3 apply_fog(vec3 color, float dist, vec3 cam_pos, vec3 cam_dir) {
    const float a = 0.00001f; // some parameter
    const float b = 0.2f; // fog density
    float fog_amount = (a / b) * exp(-cam_pos.y * b) * (1.0 - exp(-dist * cam_dir.y * b)) / cam_dir.y;
    const vec3 fog_color = vec3(0.5, 0.6, 0.7);
    return mix(color, fog_color, fog_amount);
}

vec3 reinhard_tonemap(vec3 hdr) {
    vec3 ldr = hdr / (hdr + vec3(1.0));
    return ldr;
}

vec3 exposure_tonemap(vec3 hdr, float exposure) {
    // Exposure tone mapping
    return vec3(1.0) - exp(-hdr * exposure);
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
    color = apply_fog(color, length(CamPos - FragPos), CamPos, CamPos - FragPos);
    color = exposure_tonemap(color, 0.3);

    FragColor = vec4(color, 1);
}