// pipeline.c

#include <rxcore/rendering/pipeline.h>
#include <rxcore/rendering/camera.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering.h>
#include <rxcore/rendering/shader.h>

rxcore_pipeline_t *rxcore_pipeline_create(gs_graphics_pipeline_desc_t pipeline_desc)
{
    rxcore_pipeline_t *pipeline = malloc(sizeof(rxcore_pipeline_t));
    pipeline->pipeline_hndl = gs_graphics_pipeline_create(&pipeline_desc);
    pipeline->render_passes = NULL;
    pipeline->render_pass_data = NULL;
    pipeline->render_pass_count = 0;
    pipeline->current_shader_set = (rxcore_shader_set_t){0};
    pipeline->current_material = (rxcore_material_t){0};
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
                    RXCORE_SHADER_SET_UNLIT_DEFAULT
                )
            )->program),

        },
        .layout = {
            .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_position"},
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_normal"},
                {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"},
            },
            .size = 3 * sizeof(gs_graphics_vertex_attribute_desc_t),
        },
    };

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

void rxcore_pipeline_render(rxcore_rendering_context_t *ctx)
{
    rxcore_pipeline_t *pipeline = ctx->pipeline;
    gs_command_buffer_t *cb = ctx->cb;
    gs_vec2 fs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    gs_graphics_renderpass_begin(cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
    gs_graphics_pipeline_bind(cb, pipeline->pipeline_hndl);
    gs_graphics_set_viewport(cb, 0, 0, fs.x, fs.y);
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}}};
    gs_graphics_clear(cb, &clear);

    gs_println("Rendering pipeline");

    // now create the bindings for the meshes
    rxcore_mesh_buffer_apply_bindings(ctx->mesh_registry->buffer, cb);
    // now create the bindings for the cameras
    rxcore_camera_apply_bindings(ctx->camera, cb);

    gs_println("Pipeline camera and meshes bound");

    // now traverse the scene graph to draw meshes
    rxcore_scene_graph_traverse(ctx->scene_graph, rxcore_pipeline_render_traversal, ctx);
    gs_graphics_renderpass_end(cb);
    // now execute the render passes
    gs_graphics_command_buffer_submit(cb);
    gs_println("Pipeline rendered");
}

void rxcore_pipeline_destroy(rxcore_pipeline_t *pipeline)
{
    gs_graphics_pipeline_destroy(pipeline->pipeline_hndl);
    free(pipeline->render_passes);
    free(pipeline->render_pass_data);
    free(pipeline);
}

void rxcore_pipeline_render_traversal(rxcore_scene_node_t *node, gs_mat4 model_matrix, int depth, void *user_data)
{
    gs_println("Rendering node at depth %d", depth);
    rxcore_rendering_context_t *ctx = (rxcore_rendering_context_t *)user_data;
    gs_command_buffer_t *cb = ctx->cb;

    if (node->material == NULL || rxcore_mesh_is_empty(&node->mesh))
    {
        return;
    }

    gs_println("Rendering node with mesh and material");

    // check we are not using the same shader set
    // if (ctx->pipeline->current_shader_set != node->material->shader_set)
    {
        // we need to bind the shader
        ctx->pipeline->current_shader_set = node->material->shader_set;
        // rxcore_shader_program_set(node->material->shader_set);
    }

    gs_println("Shader set bound");

    // are we using the same material?
    // if (ctx->pipeline->current_material != node->material)
    {
        // we need to bind the material
        rxcore_material_bind(node->material, cb);
    }

    gs_println("Material bound");



    // now draw the mesh
    rxcore_mesh_draw(&node->mesh, cb);
}