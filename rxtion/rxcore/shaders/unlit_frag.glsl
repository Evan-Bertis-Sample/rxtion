#version 330 core
precision mediump float;
// unlit_frag.glsl

#include "rxcore_shader_frag_unlit_util"

void main()
{
    vec2 uv = v_uv;
    frag_color = vec4(uv.x * u_color.x, uv.y * u_color.y, u_color.z, 1.0);
    frag_color.r = 1.0;
}