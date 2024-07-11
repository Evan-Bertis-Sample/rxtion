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
    gs_println("rxcore_rendering_init()");
    g_rendering_context = rxcore_rendering_context_create();

    RXCORE_PROFILER_BEGIN_TASK("shader_loading");
    _rxcore_rendering_load_core_shader_dependencies(g_rendering_context.shader_registry);
    _rxcore_rendering_load_core_shaders(g_rendering_context.shader_registry);
    rxcore_shader_registry_write_compiled_shaders_to_file(g_rendering_context.shader_registry, "bin/compiled_shaders");
    RXCORE_PROFILER_END_TASK();

    RXCORE_PROFILER_BEGIN_TASK("material_loading");
    _rxcore_rendering_load_core_material_prorotypes(g_rendering_context.material_registry, g_rendering_context.shader_registry);
    _rxcore_rendering_load_core_materials(g_rendering_context.material_registry);
    RXCORE_PROFILER_END_TASK();


    RXCORE_PROFILER_BEGIN_TASK("scene_loading");
    RXCORE_MESH_PRIMATIVE_ADD(g_rendering_context.mesh_registry, rxcore_mesh_primatives_quad, "quad");
    RXCORE_MESH_PRIMATIVE_ADD(g_rendering_context.mesh_registry, rxcore_mesh_primatives_triangle, "triangle");
    rxcore_scene_graph_add_child(
        g_rendering_context.scene_graph,
        rxcore_scene_node_create(
            rxcore_transform_empty(),
            rxcore_mesh_registry_get_mesh(g_rendering_context.mesh_registry, "quad"),
            rxcore_material_registry_get_material(g_rendering_context.material_registry, "unlit")
        )
    );
    // rxcore_scene_graph_add_child(
    //     g_rendering_context.scene_graph,
    //     rxcore_scene_node_create(
    //         rxcore_transform_create(gs_v3(0.f, 2.f, 5.f), gs_v3(1.f, 1.f, 1.f), gs_quat_default()),
    //         rxcore_mesh_registry_get_mesh(g_rendering_context.mesh_registry, "quad"),
    //         rxcore_material_registry_get_material(g_rendering_context.material_registry, "unlit")
    //     )
    // );

    RXCORE_PROFILER_END_TASK();

    RXCORE_PROFILER_BEGIN_TASK("camera_loading");
    g_rendering_context.camera = rxcore_camera_create_perspective(
        (rxcore_camera_perspective_desc_t){
            .fov = 45.f,
            .aspect_ratio = 16.f / 9.f,
            .near_plane = 0.1f,
            .far_plane = 100.f
        },
        gs_vec3_ctor(0.f, 0.f, 5.f),
        gs_quat_default()
    );
    RXCORE_PROFILER_END_TASK();
    RXCORE_PROFILER_END_TASK();
}

void rxcore_rendering_update()
{
    g_rendering_context.camera->position.x = sin(gs_platform_elapsed_time()) * 5.f;
    rxcore_pipeline_render(&g_rendering_context);
}

void rxcore_rendering_shutdown()
{
    rxcore_rendering_context_destroy(&g_rendering_context);
}

rxcore_rendering_context_t rxcore_rendering_context_create()
{
    rxcore_rendering_context_t context = {0};
    context.shader_registry = rxcore_shader_registry_create();
    context.material_registry = rxcore_material_registry_create();
    context.mesh_registry = rxcore_mesh_registry_create();
    context.scene_graph = rxcore_scene_graph_create();
    gs_command_buffer_t *cb = malloc(sizeof(gs_command_buffer_t));
    *cb = gs_command_buffer_new();
    context.cb = cb;
    context.pipeline = rxcore_pipeline_default();

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

void _rxcore_rendering_load_core_shader_dependencies(rxcore_shader_registry_t *reg)
{
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_VERT_UTIL_HANDLE,
        CORE_ASSET("shaders/util/vertex_util.glsl")
    );
    rxcore_shader_registry_add_dependency(reg,
        RXCORE_SHADER_FRAG_UTIL_HANDLE,
        CORE_ASSET("shaders/util/fragment_util.glsl")
    );
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_FRAG_LIT_UTIL_HANDLE,
        CORE_ASSET("shaders/util/fragment_lit_util.glsl")
    );
    rxcore_shader_registry_add_dependency(reg, 
        RXCORE_SHADER_FRAG_UNLIT_UTIL_HANDLE,
        CORE_ASSET("shaders/util/fragment_unlit_util.glsl")
    );
}

void _rxcore_rendering_load_core_shaders(rxcore_shader_registry_t *reg)
{
    rxcore_shader_desc_t default_vert_desc = RXCORE_SHADER_DESC(
        RXCORE_SHADER_VERTEX,
        CORE_ASSET("shaders/vertex_default.glsl"),
        RXCORE_SHADER_STAGE_VERTEX
    );
    rxcore_shader_registry_add_shader(reg, default_vert_desc);

    rxcore_shader_desc_t default_frag_lit_desc = RXCORE_SHADER_DESC(
        RXCORE_SHADER_FRAG_LIT,
        CORE_ASSET("shaders/lit_frag.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT
    );
    rxcore_shader_registry_add_shader(reg, default_frag_lit_desc);

    rxcore_shader_desc_t default_frag_unlit_desc = RXCORE_SHADER_DESC(
        RXCORE_SHADER_FRAG_UNLIT,
        CORE_ASSET("shaders/unlit_frag.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT
    );
    rxcore_shader_registry_add_shader(reg, default_frag_unlit_desc);
}


void _rxcore_rendering_load_core_material_prorotypes(rxcore_material_registry_t *reg, rxcore_shader_registry_t *shader_reg)
{
    rxcore_shader_set_t lit_shader_set = rxcore_shader_registry_get_shader_set(shader_reg, RXCORE_SHADER_SET_LIT_DEFAULT);
    rxcore_material_prototype_t lit_prototype = RXCORE_MATERIAL_PROTOTYPE_CREATE(
        lit_shader_set,
        RXCORE_UNIFORM_DESC("u_color", RXCORE_SHADER_STAGE_FRAGMENT, GS_GRAPHICS_UNIFORM_VEC4)
    );
    rxcore_material_registry_add_prototype(reg, "lit", &lit_prototype);

    rxcore_shader_set_t unlit_shader_set = rxcore_shader_registry_get_shader_set(shader_reg, RXCORE_SHADER_SET_UNLIT_DEFAULT);

    rxcore_material_prototype_t unlit_prototype = RXCORE_MATERIAL_PROTOTYPE_CREATE(
        unlit_shader_set,
        RXCORE_UNIFORM_DESC("u_color", RXCORE_SHADER_STAGE_FRAGMENT, GS_GRAPHICS_UNIFORM_VEC4)
    );
    rxcore_material_registry_add_prototype(reg, "unlit", &unlit_prototype);
}

void _rxcore_rendering_load_core_materials(rxcore_material_registry_t *reg)
{
    rxcore_material_t *lit_material = RXCORE_MATERIAL_CREATE_FROM_PROTOTYPE(
        rxcore_material_registry_get_prototype(reg, "lit")
    );
    rxcore_material_add_binding(lit_material, "u_color", &(gs_vec4){1.f, 0.f, 0.f, 1.f}, sizeof(gs_vec4));
    rxcore_material_registry_add_material(reg, "lit", lit_material);

    rxcore_material_t *unlit_material = RXCORE_MATERIAL_CREATE_FROM_PROTOTYPE(
        rxcore_material_registry_get_prototype(reg, "unlit")
    );
    rxcore_material_add_binding(unlit_material, "u_color", &(gs_vec4){1.f, 1.f, 1.f, 1.f}, sizeof(gs_vec4));
    rxcore_material_registry_add_material(reg, "unlit", unlit_material);
}