#version 330
precision mediump float;
// unlit_frag.glsl

#include "rxcore_shader_frag_unlit_util"

void main()
{
    vec2 uv = v_uv;
    FragColor = vec4(uv.x * u_color.x, uv.y * u_color.y, u_color.z, 1.0);
}