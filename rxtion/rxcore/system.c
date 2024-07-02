// system.c

#include <rxcore/system.h>

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
