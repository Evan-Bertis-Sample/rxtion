#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <gs/gs.h>

typedef struct rxcore_transform_t
{
    gs_vec3 position;
    gs_vec3 scale;
    gs_quat rotation;
} rxcore_transform_t;


rxcore_transform_t transform_create(gs_vec3 position, gs_vec3 scale, gs_quat rotation);
gs_mat4 rxcore_transform_to_mat4(rxcore_transform_t *transform);

// will probably add more functions here later...


#endif // __TRANSFORM_H__