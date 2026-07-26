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
void lowlib_set_border(unsigned char);

enum WINDOW_RESIZE_STATES {
    LOWLIB_WINDOW_NORMAL = 'N',
    LOWLIB_WINDOW_FULLSCREEN = 'F',
    LOWLIB_WINDOW_MAXIMIZED = 'M'
};

#define XCURSOR_LEFT_POINTER "left_ptr"
#define XCURSOR_GRAB "fleur"

#define XCURSOR_HORIZONTAL_RESIZE "sb_h_double_arrow"
#define XCURSOR_VERTICAL_RESIZE "sb_v_double_arrow"
#define XCURSOR_NW_RESIZE "top_left_corner"
#define XCURSOR_NE_RESIZE "top_right_corner"
#define XCURSOR_SW_RESIZE "bottom_left_corner"
#define XCURSOR_SE_RESIZE "bottom_right_corner"

#define XCURSOR_NWSE_RESIZE "nwse-resize"
#define XCURSOR_NESW_RESIZE "nesw-resize"
#define XCURSOR_LEFT_RIGHT_RESIZE "ew-resize"
#define XCURSOR_UP_DOWN_RESIZE "ns-resize"
