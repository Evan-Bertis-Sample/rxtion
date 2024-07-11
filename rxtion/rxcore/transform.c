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

rxcore_transform_t rxcore_transform_empty()
{
    rxcore_transform_t t = {0};
    t.position = gs_vec3_ctor(0, 0, 0);
    t.scale = gs_vec3_ctor(1, 1, 1);
    t.rotation = gs_quat_default();
}

gs_mat4 rxcore_transform_to_mat4(rxcore_transform_t *transform)
{
    gs_vqs vqs = gs_vqs_ctor(transform->position, transform->rotation, transform->scale);
    return gs_vqs_to_mat4(&vqs);
}
