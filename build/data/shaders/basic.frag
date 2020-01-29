#version 450 core

in vec2 TexCoords;
in vec3 Normal;
in float Height;
in vec3 FragPos;

in vec4 CamPosViewSpace;

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

vec3 apply_fog( vec3  rgb,  // original color of the pixel
     float dist,            // camera to point distance
     vec3  rayOri,              // camera position
     vec3  rayDir)              // camera to point vector
{
    const float maxFogHeight = 15; // higher the fog won't go
    const float c = 0.1;
    const float b = 0.2;

    // if this is not done, the result will look awful
    if(FragPos.y >= maxFogHeight - 1 / c) {		
        return rgb;
    }

    // distance in fog is calculated with a simple intercept theorem
    float distInFog = dist * (maxFogHeight - FragPos.y) / (FragPos.y - CamPos.y);

    // when dist is 0, log(dist) is 1, so subtract this
    float fogAmount = 1 - (log(distInFog * c) - 1) * b;

    // at the top border, the value can get greater than 1, so clamp
    fogAmount = clamp(fogAmount, 0, 1);

    // float fogAmount = c * exp(-rayOri.y*b) * (1.0 - exp(-distance*rayDir.y*b)) / rayDir.y;
    // fogAmount = clamp(fogAmount, 0, 1);
    const vec3 fog_color = vec3(0.5, 0.6, 0.7);
    return mix(fog_color, rgb, fogAmount);
}


vec3 new_fog(vec3 color) {
    const float density = 0.03f;
    float coord = abs(CamPosViewSpace.z / CamPosViewSpace.w);
    float fact = exp(-pow(density * coord, 2.0));
    fact = 1 - clamp(fact, 0, 1);
    const vec3 fog_color = vec3(0.5, 0.6, 0.7);
    return mix(color, fog_color, fact);
}

vec3 reinhard_tonemap(vec3 hdr) {
    vec3 ldr = hdr / (hdr + vec3(1.0));
    return ldr;
}

vec3 exposure_tonemap(vec3 hdr, float exposure) {
    // Exposure tone mapping
    return vec3(1.0) - exp(-hdr * exposure);
}

const float near = 0.1f;
const float far = 500.0f;

float linearize_depth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
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

    const float depth = linearize_depth(gl_FragCoord.z) / far;
//    color = apply_fog(color, length(FragPos - CamPos), CamPos, normalize(FragPos - CamPos));
//    color = new_fog(color);
    color = exposure_tonemap(color, 0.3);

    FragColor = vec4(color, 1);
}