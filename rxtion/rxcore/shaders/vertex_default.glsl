// vertex_default.glsl

#include "rxcore_shader_vert_util"

void main()
{
    
    vec4 pos = u_projection * u_view * u_model * vec4(a_position, 1.0);
    if (pos.w == 0.0) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    pos = vec4(u_uv.x * 2.0 - 1.0, u_uv.y, 0.0, 1.0);
    gl_Position = pos;

    rxcore_send_out();
}