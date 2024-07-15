#ifndef __PIPELINE_H__
#define __PIPELINE_H__

#include <gs/gs.h>
#include <rxcore/profiler.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/scene_graph.h>

typedef struct rxcore_render_pass_t rxcore_render_pass_t;
typedef struct rxcore_rendering_context_t rxcore_rendering_context_t;

enum rxcore_render_pass_type_t
{
    RXCORE_BEFORE_SCENE_RENDER,
    RXCORE_AFTER_SCENE_RENDER,
};

typedef struct rxcore_render_pass_t
{
    enum rxcore_render_pass_type_t type;
    void (*begin)(gs_command_buffer_t *cb, rxcore_render_pass_t *pass, void *data);
    void (*execute)(gs_command_buffer_t *cb, rxcore_render_pass_t *pass, void *data);
    void (*end)(gs_command_buffer_t *cb, rxcore_render_pass_t *pass, void *data);
} rxcore_render_pass_t;

typedef struct rxcore_pipeline_t
{
    gs_handle(gs_graphics_pipeline_t) pipeline_hndl;
    rxcore_render_pass_t *render_passes;
    void **render_pass_data;
    uint32_t render_pass_count;
    // state variables
    rxcore_shader_set_t current_shader_set;
    rxcore_material_t current_material;
} rxcore_pipeline_t;

rxcore_pipeline_t *rxcore_pipeline_create(gs_graphics_pipeline_desc_t pipeline_desc);
rxcore_pipeline_t *rxcore_pipeline_default();
rxcore_pipeline_t *rxcore_pipeline_add_render_pass(rxcore_pipeline_t *pipeline, rxcore_render_pass_t pass, void *data);
void rxcore_pipeline_begin(rxcore_rendering_context_t *ctx);
void rxcore_pipeline_render(rxcore_rendering_context_t *ctx);
void rxcore_pipeline_destroy(rxcore_pipeline_t *pipeline);

void rxcore_pipeline_render_traversal(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data);

#endif // __PIPELINE_H__