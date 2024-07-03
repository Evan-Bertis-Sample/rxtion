// rendering.c

#include <rxcore/rendering.h>
#include <gs/gs.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering/mesh_primatives.h>
#include <rxcore/rendering/scene_graph.h>
#include <rxcore/transform.h>

#include <rxcore/profiler.h>

void rxcore_rendering_init()
{
    RXCORE_PROFILER_BEGIN_TASK("rxcore_rendering_init");
    // gs_println("rxcore_rendering_init()");
    g_rendering_context = rxcore_rendering_context_create();

    RXCORE_PROFILER_BEGIN_TASK("load_shader_dependencies");
    _rxcore_rendering_load_core_shader_dependencies(g_rendering_context.shader_registry);
    RXCORE_PROFILER_END_TASK();
    _rxcore_rendering_load_core_shaders(g_rendering_context.shader_registry);

    RXCORE_PROFILER_END_TASK();
}

void rxcore_rendering_update()
{

}

void rxcore_rendering_shutdown()
{
    rxcore_rendering_context_destroy(&g_rendering_context);
}

void _rxcore_rendering_load_core_shader_dependencies(rxcore_shader_registry_t *reg)
{
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_VERT_UTIL_HANDLE,
        CORE_ASSET("shaders/vertex_util.glsl")
    );
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_FRAG_LIT_HANDLE,
        CORE_ASSET("shaders/fragment_lit_util.glsl")
    );
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_FRAG_UNLIT_HANDLE,
        CORE_ASSET("shaders/fragment_unlit_util.glsl")
    );
}

void _rxcore_rendering_load_core_shaders(rxcore_shader_registry_t *reg)
{
    rxcore_shader_desc_t default_vert_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        RXCORE_SHADER_VERTEX_DEFAULT,
        CORE_ASSET("shaders/vertex_default.glsl"),
        RXCORE_SHADER_STAGE_VERTEX,
        RXCORE_SHADER_VERT_UTIL_HANDLE
    );
    rxcore_shader_registry_add_shader(reg, default_vert_desc);

    rxcore_shader_desc_t default_frag_lit_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        RXCORE_SHADER_FRAGMENT_LIT_DEFAULT,
        CORE_ASSET("shaders/fragment_lit_default.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT,
        RXCORE_SHADER_FRAG_LIT_HANDLE
    );
    rxcore_shader_registry_add_shader(reg, default_frag_lit_desc);

    rxcore_shader_desc_t default_frag_unlit_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        RXCORE_SHADER_FRAGMENT_UNLIT_DEFAULT,
        CORE_ASSET("shaders/fragment_unlit_default.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT,
        RXCORE_SHADER_FRAG_UNLIT_HANDLE
    );
    rxcore_shader_registry_add_shader(reg, default_frag_unlit_desc);
}

rxcore_rendering_context_t rxcore_rendering_context_create()
{
    rxcore_rendering_context_t context = {0};
    context.shader_registry = rxcore_shader_registry_create();
    context.material_registry = rxcore_material_registry_create();
    context.mesh_registry = rxcore_mesh_registry_create();
    context.scene_graph = rxcore_scene_graph_create();
    return context;
}

void rxcore_rendering_context_destroy(rxcore_rendering_context_t *context)
{
    rxcore_shader_registry_destroy(context->shader_registry);
    rxcore_material_registry_destroy(context->material_registry);
    rxcore_mesh_registry_destroy(context->mesh_registry);
    rxcore_scene_graph_destroy(context->scene_graph);
    free(context);
}