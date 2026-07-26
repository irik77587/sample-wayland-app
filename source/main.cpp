#include "lowlib.hpp"

// if resize is set to false, then restore, maximize, minimize, fullscreen are all disabled.

lowlib_color color_back = {0xfd, 0xc4, 0x7c};
lowlib_color color_fore = {0xbd, 0xac, 0x5d};

unsigned short x_max = 480, y_max = 360;

lowlib_color shader(unsigned short x, unsigned short y, unsigned short x_max_now, unsigned short y_max_now)
{
    if (x_max_now > 0) x_max = x_max_now;
    if (y_max_now > 0) y_max = y_max_now;
    return color_fore;
}

int margin = 5;
const char * cursor(int x, int y)
{
    return XCURSOR_LEFT_POINTER;
}

void lowlib_set_bootup()
{
    lowlib_set_window(x_max, y_max);
    lowlib_set_resize(true);
    lowlib_set_cursor(cursor);
    lowlib_set_shader(shader);
}
