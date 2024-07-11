#ifndef __BOUNDING_BOX_H__
#define __BOUNDING_BOX_H__

#include <gs/gs.h>

typedef struct rxcore_bounding_box_t
{
    gs_vec3 min;
    gs_vec3 max;
} rxcore_bounding_box_t;

rxcore_bounding_box_t rxcore_bounding_box_create(gs_vec3 min, gs_vec3 max);
rxcore_bounding_box_t rxcore_bounding_box_empty();
gs_vec3 rxcore_bounding_box_get_center(rxcore_bounding_box_t *box);
gs_vec3 rxcore_bounding_box_get_size(rxcore_bounding_box_t *box);
gs_vec3 rxcore_bounding_box_get_extents(rxcore_bounding_box_t *box);
void rxcore_bounding_box_encapsulate_point(rxcore_bounding_box_t *box, gs_vec3 point);
bool rxcore_bounding_box_contains_point(rxcore_bounding_box_t *box, gs_vec3 point);
bool rxcore_bounding_box_contains_box(rxcore_bounding_box_t *box, rxcore_bounding_box_t *other);
bool rxcore_bounding_box_intersects_box(rxcore_bounding_box_t *box, rxcore_bounding_box_t *other);
rxcore_bounding_box_t rxcore_bounding_box_transform(rxcore_bounding_box_t *box, gs_mat4 transform);


#endif // __BOUNDING_BOX_H__