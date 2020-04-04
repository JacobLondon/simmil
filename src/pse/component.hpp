#pragma once

#include "types.hpp"

namespace pse {

struct component {

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bool mouse_hovering = false;
    bool mouse_pressing = false;
    bool mouse_pressed = false;
    bool focused = false; // invisible
    component();
    component(int x, int y, int w, int h);
    bool within(ivec2 coords);
    void reset_focus();
};

} // pse
