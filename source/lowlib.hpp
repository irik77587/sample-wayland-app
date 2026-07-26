#pragma once
/*
 * RGB color {r,g,b}
 */
struct lowlib_color {
    unsigned char r, g, b;
};

void lowlib_set_shader(lowlib_color(*shader)(unsigned short x, unsigned short y, unsigned short current_width, unsigned short current_height));
void lowlib_set_cursor(const char*(*cursor)(int x, int y));
void lowlib_set_resize(bool);
void lowlib_set_window(int width, int height);
void lowlib_set_bootup(); // Implemented to app, called by connect
char lowlib_get_window();

enum WINDOW_RESIZE_STATES {
    LOWLIB_WINDOW_NORMAL = 'N',
    LOWLIB_WINDOW_FULLSCREEN = 'F',
    LOWLIB_WINDOW_MAXIMIZED = 'M'
};

#define XCURSOR_LEFT_POINTER "left_ptr"
