// pipeline.c

#include <rxcore/rendering/pipeline.h>

rxcore_pipeline_t *rxcore_pipeline_create(gs_graphics_pipeline_desc_t pipeline_desc)
{
    rxcore_pipeline_t *pipeline = malloc(sizeof(rxcore_pipeline_t));
    pipeline->pipeline = gs_graphics_pipeline_create(&pipeline_desc);
    pipeline->render_passes = NULL;
    pipeline->render_pass_data = NULL;
    pipeline->render_pass_count = 0;
    pipeline->current_shader_set = NULL;
    pipeline->current_material = NULL;
    return pipeline;
}

rxcore_pipeline_t *rxcore_pipeline_default()
{
    gs_graphics_pipeline_desc_t pipeline_desc = {
        .raster = {
            .face_culling = GS_GRAPHICS_FACE_CULLING_BACK,
            .index_buffer_element_size = sizeof(uint32_t),
            .winding_order = GS_GRAPHICS_WINDING_ORDER_CCW,
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

void rxcore_pipeline_render(rxcore_pipeline_t *pipeline, gs_command_buffer_t *cb, rxcore_rendering_context_t *ctx)
{
    // before scene render
    for (uint32_t i = 0; i < pipeline->render_pass_count; i++)
    {
        if (pipeline->render_passes[i].type == RXCORE_BEFORE_SCENE_RENDER)
        {
            pipeline->render_passes[i].begin(cb, &pipeline->render_passes[i], pipeline->render_pass_data[i]);
            pipeline->render_passes[i].execute(cb, &pipeline->render_passes[i], pipeline->render_pass_data[i]);
            pipeline->render_passes[i].end(cb, &pipeline->render_passes[i], pipeline->render_pass_data[i]);
        }
    }    
}