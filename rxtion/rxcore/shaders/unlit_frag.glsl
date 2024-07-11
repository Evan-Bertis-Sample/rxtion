#version 330
precision mediump float;
// unlit_frag.glsl

#include "rxcore_shader_frag_unlit_util"

void main()
{
    vec2 uv = v_uv;
    FragColor = vec4(uv.x, uv.y, 0.0, 1.0);
}