#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <stdint.h>
#include <rx_core/rendering/shader.h>
#include <gs/gs.h>

typedef struct rxcore_material_prototype_t
{
    rxcore_shader_set_t shader_set;
    gs_graphics_uniform_desc_t *uniform_descs;
    uint32_t num_uniforms;
} rxcore_material_prototype_t;

typedef struct rxcore_material_t
{
    rxcore_shader_set_t shader_set;
    gs_handle(gs_graphics_uniform_t) *uniform_handles;
    gs_graphics_bind_uniform_desc_t *uniform_bindings;
    gs_hash_table(const char *, uint32_t) uniform_name_to_index;
    uint32_t num_uniforms;
} rxcore_material_t;

// RXCORE_MATERIAL_PROTOTYPE methods
rxcore_material_prototype_t rxcore_material_prototype_create_impl(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms);
#define rxcore_material_prototype_create(set, ...) rxcore_material_prototype_create_impl(set, (gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))

// RXCORE_MATERIAL methods
rxcore_material_t *rxcore_material_create(rxcore_shader_set_t set,gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms);
rxcore_material_t *rxcore_material_create(const rxcore_material_prototype_t *prototype, gs_graphics_uniform_desc_t *override_uniform_descs, uint32_t num_overrides);
void rxcore_material_add_binding(rxcore_material_t *material, rxcore_shader_stage_t stage, const char *uniform_name, void *data, uint32_t size);
void rxcore_material_bind(rxcore_material_t *material, gs_command_buffer_t *cb);
void rxcore_material_destroy(rxcore_material_t *material);

#define rx_material_create_from_prototype_overide(prototype, ...) rxcore_material_create(prototype, (gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))
#define rx_material_create_from_prototype(prototype) rxcore_material_create(prototype, NULL, 0)
#define rx_material_create(...) rxcore_material_create((gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))

#define rxcore_uniform_desc(uniform_name, uniform_stage, uniform_type) \
    { \
        .stage = uniform_stage, \
        .name = uniform_name, \
        .layout = &(gs_graphics_uniform_layout_desc_t){ \
            .type = uniform_type \
        } \
    }

#endif // __MATERIAL_H__