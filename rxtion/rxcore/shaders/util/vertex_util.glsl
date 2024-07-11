// vertex_util.glsl

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

// attributes
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

// pass to fragment shader
out vec3 v_world_position;
out vec3 v_object_position;
out vec3 v_screen_position;
out vec3 v_world_normal;
out vec3 v_object_normal;
out vec2 v_uv;


void rxcore_send_out() {
    v_world_position = (u_model * vec4(a_position, 1.0)).xyz;
    v_object_position = a_position;

    vec4 pos = u_projection * u_view * u_model * vec4(a_position, 1.0);
    if (pos.w == 0.0) {
        v_screen_position = vec3(0.0, 0.0, 0.0);
    } else {
        pos /= pos.w;
        v_screen_position = pos.xyz;
    }

    v_world_normal = (u_model * vec4(a_normal, 0.0)).xyz;
    v_object_normal = a_normal;
    v_uv = a_uv;
}