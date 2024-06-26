#ifndef __RENDERING_H__
#define __RENDERING_H__

#include <rx_core/system.h>

void rxcore_rendering_init();
void rxcore_rendering_update();
void rxcore_rendering_shutdown();

#define rxcore_rendering_system RXCORE_SYSTEM(rxcore_rendering_init, rxcore_rendering_update, rxcore_rendering_shutdown)

#endif // __RENDERING_H__