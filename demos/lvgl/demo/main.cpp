#include <chrono>
#include <thread>

#include "lvgl/lvgl.h"
#include "xlog.hpp"

int main()
{
    xlog_dbg("hello world\n");

    lv_init();

    lv_display_t *disp = lv_sdl_window_create(800, 600);

    while (true) {
        uint32_t idle_time = lv_timer_handler();
        std::this_thread::sleep_for(std::chrono::milliseconds(idle_time));
    }

    return 0;
}

