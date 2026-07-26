#include "lowlib.hpp"

// if resize is set to false, then restore, maximize, minimize, fullscreen are all disabled.

lowlib_color color_back = {0xfd, 0xc4, 0x7c};
lowlib_color color_fore = {0xbd, 0xac, 0x5d};

lowlib_color shader(unsigned short x, unsigned short y, unsigned short x_max, unsigned short y_max)
{
    return color_fore;
}

const char * cursor(int x, int y)
{
    const char * cursor = XCURSOR_LEFT_POINTER;
    return cursor;
}

void lowlib_set_bootup()
{
    lowlib_set_window(480, 360);
    lowlib_set_resize(false);
    lowlib_set_cursor(cursor);
    lowlib_set_shader(shader);
}
