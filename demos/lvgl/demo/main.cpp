#include <chrono>
#include <thread>

#include "lvgl/lvgl.h"
#include "lvgl/src/widgets/label/lv_label.h"
#include "xlog.h"

int main()
{
    xlog_dbg("hello world\n");

    lv_init();

    lv_display_t *disp = lv_sdl_window_create(800, 600);

    lv_obj_t *screen = lv_screen_active();
    xlog_dbg("screen: %p\n", screen);

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "123123");

    while (true) {
        uint32_t idle_time = lv_timer_handler();
        std::this_thread::sleep_for(std::chrono::milliseconds(idle_time));
    }

    return 0;
}

