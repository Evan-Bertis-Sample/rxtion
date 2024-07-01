#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <rx_core/system.h>
#include <gs/gs.h>
#include <rx_core/rendering/shader.h>
#include <rx_core/rendering/material.h>
#include <rx_core/rendering/mesh.h>
#include <rx_core/rendering/mesh_primatives.h>

#define CORE_ASSET(ASSET_NAME) "rxtion/rx_core/" ASSET_NAME
#define APP_ASSET(ASSET_NAME) "rxtion/rx_app/assets/" ASSET_NAME

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
    rxcore_mesh_t mesh = rxcore_mesh_registry_get_mesh(mesh_reg, "quad");
    // print out the mesh
    rxcore_vertex_t *mesh_vertices = rxcore_mesh_get_vertices(&mesh);
    uint32_t *mesh_indices = rxcore_mesh_get_indices(&mesh);

    gs_println("Quad:");
    for (uint32_t i = 0; i < mesh.index_count; i++)
    {
        gs_println("  Vertex: %f, %f, %f", mesh_vertices[i].position.x, mesh_vertices[i].position.y, mesh_vertices[i].position.z);
    }

    for (uint32_t i = 0; i < mesh.index_count; i++)
    {
        gs_println("  Index: %d", mesh_indices[i]);
    }

    // free both the vertices and indices
    free(mesh_vertices);
    free(mesh_indices);

    gs_println("\nTriangle:");
    // now do the same for the triangle
    mesh = rxcore_mesh_registry_get_mesh(mesh_reg, "triangle");
    // print out the mesh
    mesh_vertices = rxcore_mesh_get_vertices(&mesh);
    mesh_indices = rxcore_mesh_get_indices(&mesh);

    for (uint32_t i = 0; i < mesh.index_count; i++)
    {
        gs_println("  Vertex: %f, %f, %f", mesh_vertices[i].position.x, mesh_vertices[i].position.y, mesh_vertices[i].position.z);
    }

    for (uint32_t i = 0; i < mesh.index_count; i++)
    {
        gs_println("  Index: %d", mesh_indices[i]);
    }

    gs_println("Finished init");
}

void rxcore_rendering_update()
{
}

void rxcore_rendering_shutdown()
{
}

#define rxcore_rendering_system RXCORE_SYSTEM(rxcore_rendering_init, rxcore_rendering_update, rxcore_rendering_shutdown)

#endif // __RENDERING_H__