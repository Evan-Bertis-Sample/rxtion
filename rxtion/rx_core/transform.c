// transform.c

#include <gs/gs.h>
#include <rx_core/transform.h>

transform_t transform_create(gs_vec3 position, gs_vec3 scale, gs_quat rotation)
{
    transform_t t = {0};
    t.position = position;
    t.scale = scale;
    t.rotation = rotation;
    return t;
}

gs_mat4 transform_to_mat4(transform_t *transform)
{
    gs_vqs vqs = gs_vqs_ctor(transform->position, transform->rotation, transform->scale);
    return gs_vqs_to_mat4(&vqs);
}
