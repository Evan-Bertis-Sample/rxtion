// camera.c

#include <rxcore/rendering/camera.h>
#include <gs/gs.h>

rxcore_camera_t *_rxcore_camera_create_base()
{
    rxcore_camera_t *camera = malloc(sizeof(rxcore_camera_t));
    camera->framebuffer = gs_graphics_framebuffer_create(NULL);

    // Create view and projection uniforms
    camera->view_uniform = gs_graphics_uniform_create(
        &(gs_graphics_uniform_desc_t){
            .name = "u_view",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}});


    camera->projection_uniform = gs_graphics_uniform_create(
        &(gs_graphics_uniform_desc_t){
            .name = "u_projection",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}});

    

    return camera;
}

rxcore_camera_t *rxcore_camera_create_perspective(rxcore_camera_perspective_desc_t desc, gs_vec3 position, gs_quat rotation)
{
    rxcore_camera_t *camera = _rxcore_camera_create_base();
    camera->projection_type = RXCORE_CAMERA_PROJECTION_PERSPECTIVE;
    camera->perspective_desc = desc;
    camera->position = position;
    camera->rotation = rotation;
    return camera;
}

rxcore_camera_t *rxcore_camera_create_orthographic(rxcore_camera_orthographic_desc_t desc, gs_vec3 position, gs_quat rotation)
{
    rxcore_camera_t *camera = _rxcore_camera_create_base();
    camera->projection_type = RXCORE_CAMERA_PROJECTION_ORTHOGRAPHIC;
    camera->orthographic_desc = desc;
    camera->position = position;
    camera->rotation = rotation;
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

void rxcore_camera_apply_bindings(rxcore_camera_t *camera, gs_command_buffer_t *cb)
{
    camera->view_matrix = rxcore_camera_get_view_matrix(camera);
    camera->projection_matrix = rxcore_camera_get_projection_matrix(camera);

    gs_graphics_bind_uniform_desc_t view_binding = {
        .uniform = camera->view_uniform,
        .data = &camera->view_matrix,
    };

    gs_graphics_bind_uniform_desc_t projection_binding = {
        .uniform = camera->projection_uniform,
        .data = &camera->projection_matrix,
    };

    gs_graphics_bind_uniform_desc_t bindings[] = {view_binding, projection_binding};
    gs_graphics_bind_desc_t bind_desc = {
        .uniforms = {
            .desc = bindings,
            .size = sizeof(bindings),
        }
    };

    gs_graphics_apply_bindings(cb, &bind_desc);
}

void rxcore_camera_destroy(rxcore_camera_t *camera)
{
    gs_graphics_framebuffer_destroy(camera->framebuffer);
    free(camera);
}
