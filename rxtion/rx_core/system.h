#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>

// void function ptr with no args
typedef void (*rxcore_system_fn)(void);

typedef struct rxcore_system_t
{
    rxcore_system_fn init;
    rxcore_system_fn update;
    rxcore_system_fn shutdown;
} rxcore_system_t;

#define RXCORE_SYSTEM(INIT, UPDATE, SHUTDOWN) \
    (rxcore_system_t)                          \
    {                                          \
        .init = INIT,                          \
        .update = UPDATE,                      \
        .shutdown = SHUTDOWN                   \
    }

typedef struct rxcore_systems_t
{
    rxcore_system_t *systems;
    uint32_t system_count;
} rxcore_systems_t;

#define RXCORE_SYSTEMS(...)                                                                            \
    (rxcore_systems_t)                                                                                 \
    {                                                                                                  \
        .systems = ((rxcore_system_t[]){__VA_ARGS__}),                                                 \
        .system_count = (uint32_t)(sizeof((rxcore_system_t[]){__VA_ARGS__}) / sizeof(rxcore_system_t)) \
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
}

#endif // __SYSTEM_H__