#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <gs/gs.h>
#include <stddef.h>

typedef enum rxcore_camera_projection_t
{
    RXCORE_CAMERA_PROJECTION_PERSPECTIVE,
    RXCORE_CAMERA_PROJECTION_ORTHOGRAPHIC
} rxcore_camera_projection_t;

typedef struct rxcore_camera_perspective_desc_t
{
    float fov;
    float aspect_ratio;
    float near_plane;
    float far_plane;
} rxcore_camera_perspective_desc_t;

typedef struct rxcore_camera_orthographic_desc_t
{
    float left;
    float right;
    float bottom;
    float top;
    float near_plane;
    float far_plane;
} rxcore_camera_orthographic_desc_t;

typedef struct rxcore_camera_t
{
    rxcore_camera_projection_t projection_type;
    union
    {
        rxcore_camera_perspective_desc_t perspective_desc;
        rxcore_camera_orthographic_desc_t orthographic_desc;
    };
    gs_vec3 position;
    gs_quat rotation;
    gs_handle(gs_graphics_framebuffer_t) framebuffer;
    gs_handle(gs_graphics_uniform_t) view_uniform;
    gs_handle(gs_graphics_uniform_t) projection_uniform;
    gs_mat4 view_matrix;
    gs_mat4 projection_matrix;
} rxcore_camera_t;

rxcore_camera_t *_rxcore_camera_create_base();
rxcore_camera_t *rxcore_camera_create_perspective(rxcore_camera_perspective_desc_t desc, gs_vec3 position, gs_quat rotation);
rxcore_camera_t *rxcore_camera_create_orthographic(rxcore_camera_orthographic_desc_t desc, gs_vec3 position, gs_quat rotation);

gs_mat4 rxcore_camera_get_view_matrix(rxcore_camera_t *camera);
gs_mat4 rxcore_camera_get_projection_matrix(rxcore_camera_t *camera);
gs_mat4 rxcore_camera_get_view_projection_matrix(rxcore_camera_t *camera);

bool rxcore_camera_frustum_cull(gs_mat4 view_projection, gs_vec3 position, float radius);
bool rxcore_camera_frustum_cull_aabb(gs_mat4 view_projection, gs_vec3 position, gs_vec3 scale);

void rxcore_camera_apply_bindings(rxcore_camera_t *camera, gs_command_buffer_t *cb);

void rxcore_camera_destroy(rxcore_camera_t *camera);

#endif // __CAMERA_H__