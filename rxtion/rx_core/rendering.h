#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <rx_core/system.h>
#include <gs/gs.h>
#include <rx_core/rendering/shader.h>
#include <rx_core/rendering/material.h>

#define CORE_ASSET(ASSET_NAME) "rxtion/rx_core/" ASSET_NAME
#define APP_ASSET(ASSET_NAME) "rxtion/rx_app/assets/" ASSET_NAME

void rxcore_rendering_init()
{
    gs_println("rxcore_rendering_init");
    rxcore_shader_registry_t *reg = rxcore_shader_registry_create();
    rxcore_shader_registry_add_dependency(reg, "lighting", CORE_ASSET("core_shaders/lighting.glsl"));
    rxcore_shader_registry_add_dependency(reg, "pbr", CORE_ASSET("core_shaders/pbr.glsl"));

    // print out the dependencies
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(reg->dependencies);
        gs_hash_table_iter_valid(reg->dependencies, it);
        gs_hash_table_iter_advance(reg->dependencies, it)
    )
    {
        const char *key = gs_hash_table_iter_getk(reg->dependencies, it);
        rxcore_shader_t *value = gs_hash_table_iter_get(reg->dependencies, it);

        gs_println("Dependency: %s", key);
        gs_println("Dependency Source: %s", value->shader_src);
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
    for (
        gs_hash_table_iter it = gs_hash_table_iter_new(reg->shaders);
        gs_hash_table_iter_valid(reg->shaders, it);
        gs_hash_table_iter_advance(reg->shaders, it)
    )
    {
        const char *key = gs_hash_table_iter_getk(reg->shaders, it);
        rxcore_shader_t *value = gs_hash_table_iter_get(reg->shaders, it);

        gs_println("Shader: %s", key);
        gs_println("Shader Source: %s", value->shader_src);
    }

    gs_println("Finished init");


    gs_println("creating materials");
    rxcore_shader_set_t set = rxcore_shader_registry_get_shader_set(reg, "test_shader", "test_shader");
    gs_graphics_uniform_desc_t uni_test = rxcore_uniform_desc("u_test", RXCORE_SHADER_STAGE_FRAGMENT, GS_UNIFORM_FLOAT4);

    rxcore_material_prototype_t prototype = rxcore_material_prototype_create(
        set, 
        rxcore_uniform_desc("u_color", GS_UNIFORM_FLOAT4, 0),
    );
}

void rxcore_rendering_update()
{

}

void rxcore_rendering_shutdown()
{
}

#define rxcore_rendering_system RXCORE_SYSTEM(rxcore_rendering_init, rxcore_rendering_update, rxcore_rendering_shutdown)

#endif // __RENDERING_H__