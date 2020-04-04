#include "component.hpp"

namespace pse {

component::component()
{

}

component::component(int x, int y, int w, int h)
: x{x}, y{y}, w{w}, h{h},
  mouse_hovering{false}, mouse_pressing{false}, mouse_pressed{false}, focused{false}
{

}

bool component::within(ivec2 coords)
{
    return (coords.x <= x + w &&
            coords.x > x &&
            coords.y <= y + h &&
            coords.y > y);
}

void component::reset_focus()
{
    mouse_hovering = false;
    mouse_pressing = false;
    mouse_pressed = false;
    focused = false;
}

} // pse