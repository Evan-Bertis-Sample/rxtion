// fragment_util.glsl

out vec4 frag_color;

// generic unifroms
uniform vec4 u_color;
// in from vertex shader
in vec3 v_world_position;
in vec3 v_object_position;
in vec3 v_world_normal;
in vec3 v_object_normal;
in vec2 v_uv;
in vec3 v_screen_position;