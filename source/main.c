#define GS_IMPL
#include <gs/gs.h>

#include <rx_app/app.h>

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
	return (gs_app_desc_t) {
        .user_data = gs_malloc_init(rx_app_t),
		.init = rx_app_init,
		.update = rx_app_update,
        .shutdown = rx_app_shutdown,
        .window = {
            .width = 800,
            .height = 600, 
            .frame_rate = 60
        }
	};
}
