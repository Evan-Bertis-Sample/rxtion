// vertex_default.glsl
// requires:
// vertex_util.glsl

#include "vertex_util.glsl"

void main()
{
    
    vec4 pos = u_projection * u_view * u_model * vec4(a_position, 1.0);
    if (pos.w == 0.0) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    pos /= pos.w;
    gl_Position = pos;

    rxcore_send_varyings();
}