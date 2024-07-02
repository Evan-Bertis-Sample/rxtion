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

rxcore_systems_t *rxcore_systems_create(rxcore_system_t *systems, uint32_t system_count);
#define RXCORE_SYSTEMS(...)                 \
    rxcore_systems_create(                  \
        ((rxcore_system_t[]){__VA_ARGS__}), \
        (uint32_t)(sizeof((rxcore_system_t[]){__VA_ARGS__}) / sizeof(rxcore_system_t)))


void rxcore_systems_destroy(rxcore_systems_t *core);

void rxcore_init(rxcore_systems_t *core);
void rxcore_update(rxcore_systems_t *core);
void rxcore_shutdown(rxcore_systems_t *core);
#endif // __SYSTEM_H__