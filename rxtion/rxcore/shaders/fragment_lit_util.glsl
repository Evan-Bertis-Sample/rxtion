// fragment_lit_util.glsl

// Lighting Setup
struct Light {
    vec3 position;
    vec3 diffuse_color;
    vec3 specular_color;
    float intensity;
    int type; // 0 = directional, 1 = point
}

const int MAX_LIGHTS = 4;

uniform Light u_lights[MAX_LIGHTS];
uniform int u_num_lights;
uniform int u_lighting_enabled;

// Base Material Properties
uniform vec4 u_color;