#ifndef __APP_H__
#define __APP_H__

#include <gs/gs.h>
#include <rxcore/system.h>
#include <rxcore/rendering/shader.h>
#include <rxcore/rendering.h>
#include <rxcore/profiler.h>

typedef struct rxapp_t
{
    gs_command_buffer_t cb;
} rxapp_t;

gs_global rxcore_systems_t *g_core_systems;
gs_global rxcore_systems_t *g_debug_systems;

void rxapp_init()
{
    g_debug_systems = RXCORE_SYSTEMS(
        rxcore_profiling_system,
    );

    g_core_systems = RXCORE_SYSTEMS(
        rxcore_rendering_system,
    );

    rxcore_init(g_debug_systems);

    RXCORE_PROFILER_BEGIN_TASK("rxapp_init");
    rxcore_init(g_core_systems);
    RXCORE_PROFILER_END_TASK();
    RXCORE_PROFILER_REPORT();
}

void rxapp_update()
{
    rxcore_update(g_debug_systems);
    rxcore_update(g_core_systems);
}

void rxapp_shutdown()
{
    rxcore_shutdown(g_debug_systems);
    rxcore_shutdown(g_core_systems);
}

#endif // __APP_H__