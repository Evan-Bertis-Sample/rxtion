// pipeline.c

#include <rxcore/rendering/pipeline.h>
#include <rxcore/rendering/camera.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/render_group.h>

rxcore_pipeline_t *rxcore_pipeline_create(gs_graphics_pipeline_desc_t pipeline_desc)
{
    rxcore_pipeline_t *pipeline = malloc(sizeof(rxcore_pipeline_t));
    pipeline->pipeline_hndl = gs_graphics_pipeline_create(&pipeline_desc);
    pipeline->render_passes = NULL;
    pipeline->render_pass_data = NULL;
    pipeline->render_pass_count = 0;
    return pipeline;
}

rxcore_pipeline_t *rxcore_pipeline_default(rxcore_shader_registry_t *shader_registry)
{
    gs_graphics_pipeline_desc_t pipeline_desc = {
        .raster = {
            .face_culling = GS_GRAPHICS_FACE_CULLING_BACK,
            .index_buffer_element_size = sizeof(uint32_t),
            .winding_order = GS_GRAPHICS_WINDING_ORDER_CCW,
            .shader = (rxcore_shader_program_set(
                           rxcore_shader_registry_get_shader_set(
                               shader_registry,
                               RXCORE_SHADER_SET_UNLIT_DEFAULT))
                           ->program),
            .primitive = GS_GRAPHICS_PRIMITIVE_TRIANGLES,
        },
        .layout = {
            .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_position"},
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_normal"},
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"},
            },
            .size = 3 * sizeof(gs_graphics_vertex_attribute_desc_t),
        },
        .depth = {
            .func = GS_GRAPHICS_DEPTH_FUNC_LESS,
        }};

    return rxcore_pipeline_create(pipeline_desc);
}

rxcore_pipeline_t *rxcore_pipeline_add_render_pass(rxcore_pipeline_t *pipeline, rxcore_render_pass_t pass, void *data)
{
    pipeline->render_passes = realloc(pipeline->render_passes, sizeof(rxcore_render_pass_t) * (pipeline->render_pass_count + 1));
    pipeline->render_pass_data = realloc(pipeline->render_pass_data, sizeof(void *) * (pipeline->render_pass_count + 1));
    pipeline->render_passes[pipeline->render_pass_count] = pass;
    pipeline->render_pass_data[pipeline->render_pass_count] = data;
    pipeline->render_pass_count++;
    return pipeline;
}

void rxcore_pipeline_begin(rxcore_rendering_context_t *ctx)
{

}

void rxcore_pipeline_render(rxcore_rendering_context_t *ctx)
{
    gs_command_buffer_t *cb = ctx->cb;
    rxcore_pipeline_t *pipeline = ctx->pipeline;
    gs_vec2 fs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    gs_graphics_renderpass_begin(cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
    gs_graphics_pipeline_bind(cb, pipeline->pipeline_hndl);
    gs_graphics_set_viewport(cb, 0, 0, (uint32_t)fs.x, (uint32_t)fs.y);
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}}};
    gs_graphics_clear(cb, &clear);

    // now create the bindings for the meshes
    rxcore_mesh_buffer_apply_bindings(ctx->mesh_registry->buffer, cb);

    // gs_println("Rendering pipeline");

    // now create the bindings for the cameras
    if (ctx->camera->projection_type = RXCORE_CAMERA_PROJECTION_PERSPECTIVE)
    {
        // update the aspect ratio
        ctx->camera->perspective_desc.aspect_ratio = fs.x / fs.y;
    }

    rxcore_camera_apply_bindings(ctx->camera, cb);
    // now traverse the scene graph to draw meshes
    if (ctx->scene_graph->is_dirty || ctx->render_group == NULL)
    {
        gs_println("Scene graph is dirty, updating render group");
        // free the old render_group
        if (ctx->render_group != NULL)
        {
            rxcore_render_group_destroy(ctx->render_group);
        }

        ctx->render_group = rxcore_render_group_create(ctx->scene_graph);
    }
    else
    {
        RXCORE_SCENE_GRAPH_UPDATE_MATRICES(ctx->scene_graph);
    }

    uint32_t num_render_items = gs_dyn_array_size(ctx->render_group->items);

    for (uint32_t i = 0; i < num_render_items; i++)
    {
        gs_println("Processing rendering item %d", i);
        rxcore_render_item_t item = ctx->render_group->items[i];
        switch (item.type)
        {
        case RXCORE_SWAP_ITEM:
            gs_println("Swapping material");
            gs_println("Material: %p", item.swap_item.material);
            gs_println("Shader set: %p", item.swap_item.material->shader_set);
            rxcore_shader_program_set(item.swap_item.material->shader_set);
            rxcore_material_bind(item.swap_item.material, cb);
            break;
        case RXCORE_DRAW_ITEM:
            rxcore_pipeline_render_node(cb, item.draw_item);
            break;
        }
    }

    gs_graphics_renderpass_end(cb);
    // now execute the render passes
    gs_graphics_command_buffer_submit(cb);
    // gs_println("Pipeline rendered");
}

void rxcore_pipeline_destroy(rxcore_pipeline_t *pipeline)
{
    gs_graphics_pipeline_destroy(pipeline->pipeline_hndl);
    free(pipeline->render_passes);
    free(pipeline->render_pass_data);
    free(pipeline);
}

void rxcore_pipeline_render_node(gs_command_buffer_t *cb, rxcore_draw_item_t item)
{
    // pass in the model matrix
    gs_handle(gs_graphics_uniform_t) model_binding = gs_graphics_uniform_create(
        &(gs_graphics_uniform_desc_t){
            .stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
            .name = "u_model",
            .layout = &(gs_graphics_uniform_layout_desc_t){
                .type = GS_GRAPHICS_UNIFORM_MAT4,
            },
        });

    // bind the model matrix
    gs_graphics_bind_uniform_desc_t model_desc = {
        .uniform = model_binding,
        .data = &item.node->world_matrix,
    };

    gs_graphics_bind_desc_t bind_desc = {
        .uniforms = {
            .desc = &model_desc,
        }};

    gs_graphics_apply_bindings(cb, &bind_desc);

    // now draw the mesh
    rxcore_mesh_draw(&item.node->mesh, cb);
}