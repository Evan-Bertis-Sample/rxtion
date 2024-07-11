// unlit_frag.glsl

#include "rxcore_shader_frag_unlit_util"

void main()
{
    vec2 uv = v_uv;
    gl_FragColor = float4(uv.x, uv.y, 0.0, 1.0);
}