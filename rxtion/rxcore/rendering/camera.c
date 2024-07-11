// camera.c

#include <rxcore/rendering/camera.h>
#include <gs/gs.h>

rxcore_camera_t *rxcore_camera_create_perspective(rxcore_camera_perspective_desc_t desc, gs_vec3 position, gs_quat rotation)
{
    rxcore_camera_t *camera = malloc(sizeof(rxcore_camera_t));
    camera->projection_type = RXCORE_CAMERA_PROJECTION_PERSPECTIVE;
    camera->perspective_desc = desc;
    camera->position = position;
    camera->rotation = rotation;
    camera->framebuffer = gs_graphics_framebuffer_create(NULL);
    return camera;
}

rxcore_camera_t *rxcore_camera_create_orthographic(rxcore_camera_orthographic_desc_t desc, gs_vec3 position, gs_quat rotation)
{
    rxcore_camera_t *camera = malloc(sizeof(rxcore_camera_t));
    camera->projection_type = RXCORE_CAMERA_PROJECTION_ORTHOGRAPHIC;
    camera->orthographic_desc = desc;
    camera->position = position;
    camera->rotation = rotation;
    camera->framebuffer = gs_graphics_framebuffer_create(NULL);
    return camera;
}

gs_mat4 rxcore_camera_get_view_matrix(rxcore_camera_t *camera)
{
    gs_vqs vqs = gs_vqs_ctor(camera->position, camera->rotation, gs_vec3_ctor(1, 1, 1));
    return gs_vqs_to_mat4(&vqs);
}

gs_mat4 rxcore_camera_get_projection_matrix(rxcore_camera_t *camera)
{
    switch (camera->projection_type)
    {
    case RXCORE_CAMERA_PROJECTION_PERSPECTIVE:
        return gs_mat4_perspective(camera->perspective_desc.fov, camera->perspective_desc.aspect_ratio, camera->perspective_desc.near_plane, camera->perspective_desc.far_plane);
    case RXCORE_CAMERA_PROJECTION_ORTHOGRAPHIC:
        return gs_mat4_ortho(camera->orthographic_desc.left, camera->orthographic_desc.right, camera->orthographic_desc.bottom, camera->orthographic_desc.top, camera->orthographic_desc.near_plane, camera->orthographic_desc.far_plane);
    default:
        return gs_mat4_identity();
    }
}

gs_mat4 rxcore_camera_get_view_projection_matrix(rxcore_camera_t *camera)
{
    return gs_mat4_mul(rxcore_camera_get_projection_matrix(camera), rxcore_camera_get_view_matrix(camera));
}

bool rxcore_camera_frustum_cull(gs_mat4 view_projection, gs_vec3 position, float radius)
{
    gs_vec4 sphere = gs_vec4_ctor(position.x, position.y, position.z, radius);
    gs_vec4 clip = gs_mat4_mul_vec4(view_projection, sphere);
    return clip.x + clip.w < -1 || clip.x - clip.w > 1 || clip.y + clip.w < -1 || clip.y - clip.w > 1 || clip.z + clip.w < -1 || clip.z - clip.w > 1;
}

bool rxcore_camera_frustum_cull_aabb(gs_mat4 view_projection, gs_vec3 position, gs_vec3 scale)
{
    gs_vec4 clip = gs_mat4_mul_vec4(view_projection, gs_vec4_ctor(position.x, position.y, position.z, 1));
    gs_vec3 extents = gs_vec3_scale(scale, 0.5f);
    gs_vec3 min = gs_vec3_sub(position, extents);
    gs_vec3 max = gs_vec3_add(position, extents);
    return clip.x + clip.w < -1 || clip.x - clip.w > 1 || clip.y + clip.w < -1 || clip.y - clip.w > 1 || clip.z + clip.w < -1 || clip.z - clip.w > 1;
}

