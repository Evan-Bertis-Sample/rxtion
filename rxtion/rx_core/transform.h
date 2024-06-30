#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <gs/gs.h>

typedef struct transform_t
{
    gs_vec3 position;
    gs_vec3 scale;
    gs_quat rotation;
} transform_t;


transform_t transform_create(gs_vec3 position, gs_vec3 scale, gs_quat rotation);
gs_mat4 transform_to_mat4(transform_t *transform);

// will probably add more functions here later...


#endif // __TRANSFORM_H__