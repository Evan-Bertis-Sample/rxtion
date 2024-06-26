#ifndef __APP_H__
#define __APP_H__

#include <gs/gs.h>
#include <rx_core/system.h>
#include <rx_core/rendering/shader.h>
#include <rx_core/rendering.h>


typedef struct rx_app_t
{
    gs_command_buffer_t cb;
} rx_app_t;

gs_global rxcore_systems_t *g_core_systems;

void rx_app_init()
{
    g_core_systems = RXCORE_SYSTEMS(
        rxcore_rendering_system,
    );

    rxcore_init(g_core_systems);
}

void rx_app_update()
{
    rxcore_update(g_core_systems);
}

void rx_app_shutdown()
{
    rxcore_shutdown(g_core_systems);
}

#endif // __APP_H__