// rendering.c

#include <rxcore/rendering.h>
#include <gs/gs.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering/material.h>
#include <rxcore/rendering/mesh.h>
#include <rxcore/rendering/mesh_primatives.h>
#include <rxcore/rendering/scene_graph.h>
#include <rxcore/transform.h>

void rxcore_rendering_init()
{
    gs_println("rxcore_rendering_init");
    rxcore_shader_registry_t *reg = rxcore_shader_registry_create();
    rxcore_shader_registry_add_dependency(reg, "lighting", CORE_ASSET("core_shaders/lighting.glsl"));
    rxcore_shader_registry_add_dependency(reg, "pbr", CORE_ASSET("core_shaders/pbr.glsl"));

    // print out the dependencies
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = reg->dependencies[i];
        gs_println("Dependency: %s", dep->shader_name);
    }

    // create an example shader
    rxcore_shader_desc_t shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "test_shader",
        APP_ASSET("shaders/test.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT,
        "lighting", "pbr");

    uint32_t shader_id = rxcore_shader_registry_add_shader(reg, shader_desc);

    // create an invalid shader
    rxcore_shader_desc_t invalid_shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "invalid_shader",
        APP_ASSET("shaders/invalid.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT,
        "lighting", "pbr");

    uint32_t invalid_shader_id = rxcore_shader_registry_add_shader(reg, invalid_shader_desc);

    // create a shader with missing dependencies
    rxcore_shader_desc_t missing_shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "missing_shader",
        APP_ASSET("shaders/missing.glsl"),
        RXCORE_SHADER_STAGE_FRAGMENT,
        "lighting", "pbr", "missing");

    uint32_t missing_shader_id = rxcore_shader_registry_add_shader(reg, missing_shader_desc);

    // print out the shaders
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = reg->shaders[i];
        gs_println("Shader: %s", shader->shader_name);
    }

    gs_println("Finished init");

    gs_println("creating materials");
    rxcore_shader_set_t set = rxcore_shader_registry_get_shader_set(reg, "test_shader", "test_shader");

    gs_println("Found vertex shader: %s", set.vertex_shader->shader_name);
    gs_println("Found fragment shader: %s", set.fragment_shader->shader_name);

    gs_println("Creating material prototype");
    gs_graphics_uniform_desc_t uni_test = RXCORE_UNIFORM_DESC("u_test", RXCORE_SHADER_STAGE_FRAGMENT, GS_GRAPHICS_UNIFORM_INT);

    gs_println("Creating material from prototype");
    rxcore_material_prototype_t prototype = RXCORE_MATERIAL_PROTOTYPE_CREATE(
        set,
        RXCORE_UNIFORM_DESC("u_test_float", RXCORE_SHADER_STAGE_VERTEX, GS_GRAPHICS_UNIFORM_FLOAT),
        uni_test);

    gs_println("Creating material from prototype");
    rxcore_material_t *material = RXCORE_MATERIAL_CREATE_FROM_PROTOTYPE_OVERIDE(&prototype, uni_test);

    rxcore_material_print(material);

    gs_println("Adding bindings");
    int binding = 5;
    float float_binding = 5.0f;
    rxcore_material_add_binding(material, "u_test", &binding, sizeof(int));
    rxcore_material_add_binding(material, "u_test_float", &float_binding, sizeof(float));

    gs_println("Testing meshes");

    rxcore_mesh_registry_t *mesh_reg = rxcore_mesh_registry_create();
    rxcore_vertex_t *vertices = NULL;
    uint32_t *indices = NULL;
    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    rxcore_mesh_primatives_quad(&vertices, &vertex_count, &indices, &index_count);
    rxcore_mesh_registry_add_mesh(mesh_reg, "quad", vertices, vertex_count, indices, index_count);

    // free the memory
    free(vertices);
    free(indices);

    rxcore_mesh_primatives_triangle(&vertices, &vertex_count, &indices, &index_count);
    rxcore_mesh_registry_add_mesh(mesh_reg, "triangle", vertices, vertex_count, indices, index_count);

    // free the memory
    free(vertices);
    free(indices);

    rxcore_mesh_buffer_get_vertex_buffer(mesh_reg->buffer);
    rxcore_mesh_buffer_get_index_buffer(mesh_reg->buffer);

    // get the mesh

    gs_println("Printing meshes");
    gs_println("Quad:");
    rxcore_mesh_t mesh = rxcore_mesh_registry_get_mesh(mesh_reg, "quad");
    rxcore_mesh_print(&mesh, printf, true);

    gs_println("Triangle:");
    mesh = rxcore_mesh_registry_get_mesh(mesh_reg, "triangle");
    rxcore_mesh_print(&mesh, printf, true);

    // scene graph tests
    rxcore_scene_graph_t *graph = rxcore_scene_graph_create();
    rxcore_scene_node_t *quad_child = rxcore_scene_node_create(
        rxcore_transform_create(
            gs_v3(0.0f, 0.0f, 0.0f),
            gs_v3(1.0f, 1.0f, 1.0f),
            gs_quat_default()),
        rxcore_mesh_registry_get_mesh(mesh_reg, "quad"),
        material);

    rxcore_scene_node_t *triangle_child = rxcore_scene_node_create(
        rxcore_transform_create(
            gs_v3(0.0f, 0.0f, 0.0f),
            gs_v3(1.0f, 1.0f, 1.0f),
            gs_quat_default()),
        rxcore_mesh_registry_get_mesh(mesh_reg, "triangle"),
        material);

    rxcore_scene_graph_add_child(graph, quad_child);
    rxcore_scene_graph_add_child(graph, triangle_child);

    // should be
    // root
    //   quad_child
    //       quad_child_copy
    //       quad_child_copy2
    //   triangle_child
    //       triangle_child_copy

    // now add a copy of the quad child
    rxcore_scene_node_t *quad_child_copy = rxcore_scene_node_copy(quad_child, true);
    rxcore_scene_node_add_child(quad_child, quad_child_copy);

    // now add a copy of the triangle child
    rxcore_scene_node_t *triangle_child_copy = rxcore_scene_node_copy(triangle_child, true);
    rxcore_scene_node_add_child(triangle_child, triangle_child_copy);

    // now add another copy of the quad child
    rxcore_scene_node_t *quad_child_copy2 = rxcore_scene_node_copy(quad_child, true);
    rxcore_scene_node_add_child(quad_child, quad_child_copy2);

    // print out the scene graph
    gs_println("Printing scene graph before removing quad child");
    rxcore_scene_graph_print(graph, printf);

    // now remove the quad child
    rxcore_scene_graph_remove_child(graph, quad_child);
    // print out the scene graph
    gs_println("Printing scene graph after removing quad child");
    rxcore_scene_graph_print(graph, printf);

    // destroy the scene graph
    rxcore_scene_graph_destroy(graph);
}

void rxcore_rendering_update()
{
    // gs_println("rxcore_rendering_update");
}

void rxcore_rendering_shutdown()
{
    gs_println("rxcore_rendering_shutdown");
}