#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include <stdbool.h>

#define RXCORE_SYSTEM_DEBUG

#ifdef RXCORE_SYSTEM_DEBUG
#define RXCORE_SYSTEM_DEBUG_PRINT(str, ...) gs_println("RXCORE::system::" str, __VA_ARGS__)
#else
#define RXCORE_SYSTEM_DEBUG_PRINT(...) ((void)0)
#endif

// void function ptr with no args
typedef void (*rxcore_system_fn)(void);

typedef struct rxcore_system_t
{
    rxcore_system_fn init;
    rxcore_system_fn update;
    rxcore_system_fn shutdown;
} rxcore_system_t;

#define RXCORE_SYSTEM(INIT, UPDATE, SHUTDOWN) \
    (rxcore_system_t)                         \
    {                                         \
        .init = INIT,                         \
        .update = UPDATE,                     \
        .shutdown = SHUTDOWN                  \
    }

typedef struct rxcore_systems_t
{
    rxcore_system_t *systems;
    uint32_t system_count;
} rxcore_systems_t;

rxcore_systems_t *rxcore_systems_create(rxcore_system_t *systems, uint32_t system_count)
{
    // copy systems
    rxcore_system_t *sys = (rxcore_system_t *)malloc(sizeof(rxcore_system_t) * system_count);
    memcpy(sys, systems, sizeof(rxcore_system_t) * system_count);

    // create systems
    rxcore_systems_t *core = (rxcore_systems_t *)malloc(sizeof(rxcore_systems_t));
    core->systems = sys;
    core->system_count = system_count;

    return core;
}

#define RXCORE_SYSTEMS(...)                 \
    rxcore_systems_create(                  \
        ((rxcore_system_t[]){__VA_ARGS__}), \
        (uint32_t)(sizeof((rxcore_system_t[]){__VA_ARGS__}) / sizeof(rxcore_system_t)))


void rxcore_systems_destroy(rxcore_systems_t *core)
{
    free(core->systems);
    free(core);
}

void rxcore_init(rxcore_systems_t *core)
{
    for (uint32_t i = 0; i < core->system_count; i++)
    {
        core->systems[i].init();
    }
}

void rxcore_update(rxcore_systems_t *core)
{
    for (uint32_t i = 0; i < core->system_count; i++)
    {
        core->systems[i].update();
    }
}

void rxcore_shutdown(rxcore_systems_t *core)
{
    for (uint32_t i = 0; i < core->system_count; i++)
    {
        core->systems[i].shutdown();
    }

    rxcore_systems_destroy(core);
}

#endif // __SYSTEM_H__