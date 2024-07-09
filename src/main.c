#define GS_IMPL
#include <gs/gs.h>

#include <rxcore/profiler.h>

#define RXCORE_IMPL
#include <rxapp/app.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
	return (gs_app_desc_t) {
        .user_data = gs_malloc_init(rxapp_t),
		.init = rxapp_init,
		.update = rxapp_update,
        .shutdown = rxapp_shutdown,
        .window = {
            .width = 800,
            .height = 600, 
            .frame_rate = 60,
        }
	};
}
