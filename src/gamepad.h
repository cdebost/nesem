#pragma once

#include <cstdint>

namespace nesem {

struct Gamepad {
    bool btn_a = false;
    bool btn_b = false;
    bool btn_select = false;
    bool btn_start = false;
    bool btn_up = false;
    bool btn_down = false;
    bool btn_left = false;
    bool btn_right = false;
    // Which button is currently being reported when reading
    uint8_t reporting_idx = 0;
    // Report only button A
    bool strobe = false;

    void strobe_on() {
        strobe = true;
        reporting_idx = 0;
    }

    void strobe_off() { strobe = false; }

    bool read() {
        if (strobe) {
            return btn_a;
        } else {
            bool *button = &btn_a + reporting_idx;
            ++reporting_idx;
            if (reporting_idx > 7) reporting_idx = 0;
            return *button;
        }
    }
};

}
