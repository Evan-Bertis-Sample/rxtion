// lit_frag.glsl

#include "rxcore_shader_frag_lit_util"

void main()
{
    vec3 normal = normalize(v_world_normal);
    vec3 view_dir = normalize(-v_world_position);
    vec4 color = u_color;

    if (u_lighting_enabled == 1) {
        vec3 ambient = vec3(0.1, 0.1, 0.1);
        vec3 diffuse = vec3(0.0, 0.0, 0.0);
        vec3 specular = vec3(0.0, 0.0, 0.0);

        for (int i = 0; i < u_num_lights; i++) {
            Light light = u_lights[i];
            vec3 light_dir = normalize(light.position - v_world_position);
            float diff = max(dot(normal, light_dir), 0.0);
            diffuse += light.diffuse_color * diff * light.intensity;

            vec3 reflect_dir = reflect(-light_dir, normal);
            float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
            specular += light.specular_color * spec * light.intensity;
        }

        color.rgb = color.rgb * (ambient + diffuse) + specular;
    }

    gl_FragColor = color;
}
