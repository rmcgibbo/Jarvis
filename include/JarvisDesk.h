#pragma once
#include "stdint.h"

struct JarvisDesk {
    enum movement_state_t {
        MOVE_STOP = 0,
        MOVE_UP = 1,
        MOVE_DOWN = 2
    } moving_{MOVE_STOP};

    void begin();
    void run();
    void goto_height(int16_t target_height);

    void _read_desk_serial();
    void _go_down();
    void _go_up();
    void _go_stop();

    int16_t height_{-1};
    int16_t target_height_{-1};
    bool height_changed_{false};
};
