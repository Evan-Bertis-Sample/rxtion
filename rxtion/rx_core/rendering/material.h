#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <stdint.h>
#include <rx_core/rendering/shader.h>
#include <gs/gs.h>

#define RXCORE_MATERIAL_DEBUG

#ifdef RXCORE_SHADER_DEBUG
#define RXCORE_MATERIAL_DEBUG_PRINT(str) gs_println("RXCORE::rendering::material::" str)
#define RXCORE_MATERIAL_DEBUG_PRINTF(str, ...) gs_println("RXCORE::rendering::material::" str, __VA_ARGS__)
#else
#define RXCORE_MATERIAL_DEBUG_PRINT(...) ((void)0)
#endif

typedef struct rxcore_material_prototype_t
{
    rxcore_shader_set_t shader_set;
    gs_graphics_uniform_desc_t *uniform_descs;
    uint32_t num_uniforms;
} rxcore_material_prototype_t;

typedef struct rxcore_material_t
{
    rxcore_shader_set_t shader_set;
    gs_handle(gs_graphics_uniform_t) * uniform_handles;
    gs_graphics_bind_uniform_desc_t *uniform_bindings;
    const char **uniform_name_to_index;
    uint32_t num_uniforms;
} rxcore_material_t;

// RXCORE_MATERIAL_PROTOTYPE methods
rxcore_material_prototype_t rxcore_material_prototype_create(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms);
#define RXCORE_MATERIAL_PROTOTYPE_CREATE(set, ...) rxcore_material_prototype_create(set, (gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))

// RXCORE_MATERIAL methods
rxcore_material_t *rxcore_material_create_base(rxcore_shader_set_t set, gs_graphics_uniform_desc_t *uniform_descs, uint32_t num_uniforms);
rxcore_material_t *rxcore_material_create_from_prototype(const rxcore_material_prototype_t *prototype, gs_graphics_uniform_desc_t *override_uniform_descs, uint32_t num_overrides);
void rxcore_material_add_binding(rxcore_material_t *material, const char *uniform_name, void *data, uint32_t size);
void rxcore_material_bind(rxcore_material_t *material, gs_command_buffer_t *cb);
void rxcore_material_print(rxcore_material_t *material);
bool rxcore_material_uniform_exists(rxcore_material_t *material, const char *uniform_name);
bool rxcore_material_get_uniform_index(rxcore_material_t *material, const char *uniform_name, uint32_t *out_index);
void rxcore_material_destroy(rxcore_material_t *material);

#define RXCORE_MATERIAL_CREATE_FROM_PROTOTYPE_OVERIDE(prototype, ...) rxcore_material_create_from_prototype(prototype, (gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))
#define RXCORE_MATERIAL_CREATE_FROM_PROTOTYPE(prototype) rxcore_material_create_from_prototype(prototype, NULL, 0)
#define RXCORE_MATERIAL_CREATE(...) rxcore_material_create_base((gs_graphics_uniform_desc_t[]){__VA_ARGS__}, sizeof((gs_graphics_uniform_desc_t[]){__VA_ARGS__}) / sizeof(gs_graphics_uniform_desc_t))

#define RXCORE_UNIFORM_DESC(uniform_name, uniform_stage, uniform_type) \
    {                                                                  \
        .stage = uniform_stage,                                        \
        .name = uniform_name,                                          \
        .layout = &(gs_graphics_uniform_layout_desc_t)                 \
        {                                                              \
            .type = uniform_type                                       \
        }                                                              \
    }

#endif // __MATERIAL_H__