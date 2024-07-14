// vertex_default.glsl

#include "rxcore_shader_vert_util"

void main()
{
    vec4 pos = u_projection * u_view * u_model * vec4(a_position, 1.0);
    // check that the vertex is in front of the camera
    if(pos.z < 0.0) {
        // if not, set the vertex position to the origin
        pos = vec4(0.0, 0.0, 0.0, 0.0);
    }

    pos /= pos.w;
    gl_Position = pos;

    rxcore_send_out();
}