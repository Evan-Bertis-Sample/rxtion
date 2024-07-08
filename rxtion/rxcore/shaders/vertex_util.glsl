// vertex_util.glsl

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

// attributes
attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv;

// pass to fragment shader
varying vec3 v_world_position;
varying vec3 v_object_position;
varying vec3 v_screen_position;
varying vec3 v_world_normal;
varying vec3 v_object_normal;
varying vec2 v_uv;


void rxcore_send_varyings() {
    v_world_position = (u_model * vec4(a_position, 1.0)).xyz;
    v_object_position = a_position;
    v_screen_position = rxcore_convert_object_pos_to_sceen_space(v_world_position);
    v_world_normal = (u_model * vec4(a_normal, 0.0)).xyz;
    v_object_normal = a_normal;
    v_uv = a_uv;
}