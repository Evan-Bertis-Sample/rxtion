#ifndef __APP_H__
#define __APP_H__

#include <gs/gs.h>
#include <rx_core/rendering/shader.h>

#define CORE_ASSET(ASSET_NAME) "rxtion/rx_core/" ASSET_NAME
#define APP_ASSET(ASSET_NAME) "rxtion/rx_app/" ASSET_NAME

typedef struct rx_app_t
{
    gs_command_buffer_t cb;
} rx_app_t;

void rx_app_init()
{
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
        APP_ASSET("shaders/test_shader.glsl"),
        "lighting", "pbr"
    );

    uint32_t shader_id = rxcore_shader_registry_add_shader(reg, shader_desc);

    // print all shaders
    for (uint32_t i = 0; i < gs_dyn_array_size(reg->shaders); i++)
    {
        rxcore_shader_t *shader = &reg->shaders[i];
        gs_println("Shader: %s", shader->shader_name);
        gs_println("Shader Source: %s", shader->shader_src);
    }

}

void rx_app_update()
{

}

void rx_app_shutdown()
{

}

#endif // __APP_H__