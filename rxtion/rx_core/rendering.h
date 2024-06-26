#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <rx_core/system.h>

#define CORE_ASSET(ASSET_NAME) "rxtion/rx_core/" ASSET_NAME
#define APP_ASSET(ASSET_NAME) "rxtion/rx_app/assets/" ASSET_NAME

void rxcore_rendering_init()
{
    gs_println("rxcore_rendering_init");
    rxcore_shader_registry_t *reg = rxcore_shader_registry_create();
    rxcore_shader_registry_add_dependency(reg, "lighting", CORE_ASSET("core_shaders/lighting.glsl"));
    rxcore_shader_registry_add_dependency(reg, "pbr", CORE_ASSET("core_shaders/pbr.glsl"));

    // print all dependencies
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->dependencies); i++)
    {
        rxcore_shader_t *dep = &reg->dependencies[i];
        gs_println("Dependency: %s", dep->shader_name);
    }

    // create an example shader
    rxcore_shader_desc_t shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "test_shader",
        APP_ASSET("shaders/test.glsl"),
        "lighting", "pbr");

    uint32_t shader_id = rxcore_shader_registry_add_shader(reg, shader_desc);

    // create an invalid shader
    rxcore_shader_desc_t invalid_shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "invalid_shader",
        APP_ASSET("shaders/invalid.glsl"),
        "lighting", "pbr");

    uint32_t invalid_shader_id = rxcore_shader_registry_add_shader(reg, invalid_shader_desc);

    // create a shader with missing dependencies
    rxcore_shader_desc_t missing_shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE(
        "missing_shader",
        APP_ASSET("shaders/missing.glsl"),
        "lighting", "pbr", "missing");

    uint32_t missing_shader_id = rxcore_shader_registry_add_shader(reg, missing_shader_desc);

    // print all shaders
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = &reg->shaders[i];

        if (shader == NULL)
        {
            gs_println("Shader is NULL");
            continue;
        }

        gs_println("Shader: %s", shader->shader_name);
        gs_println("Shader Source: %s", shader->shader_src);
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