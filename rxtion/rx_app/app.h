#ifndef __APP_H__
#define __APP_H__

#include <gs/gs.h>
#include <rx_core/rendering/shader.h>

typedef struct rx_app_t
{
    gs_command_buffer_t cb;
} rx_app_t;

void rx_app_init()
{
    rxcore_shader_desc_t shader_desc = RXCORE_SHADER_DESC_WITH_INCLUDE("basic", "shaders/basic.glsl", "shaders/basic_util.glsl", "shaders/basic_lighting.glsl");
}

void rx_app_update()
{

}

void rx_app_shutdown()
{

}

#endif // __APP_H__