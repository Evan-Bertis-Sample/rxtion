// transform.c

#include <gs/gs.h>
#include <rxcore/transform.h>

rxcore_transform_t rxcore_transform_create(gs_vec3 position, gs_vec3 scale, gs_quat rotation)
{
    rxcore_transform_t t = {0};
    t.position = position;
    t.scale = scale;
    t.rotation = rotation;
    return t;
}

gs_mat4 rxcore_transform_to_mat4(rxcore_transform_t *transform)
{
    gs_vqs vqs = gs_vqs_ctor(transform->position, transform->rotation, transform->scale);
    return gs_vqs_to_mat4(&vqs);
}