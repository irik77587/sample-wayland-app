#include "cursor.hpp"
#include "../lowlib.hpp"
#include "event.hpp"
#include "inc/xdg-shell.h"
#include "registry.hpp"
#include "shm.hpp"
#include <cstdint>
#include <wayland-cursor.h>
#include <wayland-util.h>
#include <linux/input-event-codes.h>

const char *fallback_set_xcursor(int x, int y);
const char *set_xcursor_resize(int x, int y);
const char *(*set_xcursor)(int x, int y) = fallback_set_xcursor;
const char *cursor_type;
uint8_t border = 5;
xdg_toplevel_resize_edge window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
static uint16_t x_max, y_max;
static xdg_toplevel*window;

wl_cursor_theme *cursor_theme;
wl_cursor_image *cursor_image;

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *, wl_fixed_t u, wl_fixed_t v) {
  auto m_app = static_cast<app_state *>(data);
  int x = wl_fixed_to_int(u), y = wl_fixed_to_int(v);
  // Start cursor loading
  cursor_theme = wl_cursor_theme_load(m_app->cursor_theme_name, 24, shm);

  cursor_type = set_xcursor_resize(x, y);
  if (x > border && y > border && x + border < x_max && y + border < y_max)
    cursor_type = set_xcursor(x, y);

  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(cursor_theme, cursor_type);
  cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, cursor_surface,
                        cursor_image->hotspot_x, cursor_image->hotspot_y);
}

void pointer_leave(void *, struct wl_pointer *, uint32_t serial,
                   struct wl_surface *) {
  if (cursor_theme == nullptr)
    return;
  wl_cursor_theme_destroy(cursor_theme);
  cursor_image = nullptr;
  cursor_theme = nullptr;
}

void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t serial,
                    wl_fixed_t u, wl_fixed_t v) {
  int x = wl_fixed_to_int(u), y = wl_fixed_to_int(v);

  cursor_type = set_xcursor_resize(x, y);
  if (x > border && y > border && x + border < x_max && y + border < y_max)
    cursor_type = set_xcursor(x, y);

  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(cursor_theme, cursor_type);
  cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, cursor_surface,
                        cursor_image->hotspot_x, cursor_image->hotspot_y);
  // Done cursor loading
}

void pointer_click(void *data, wl_pointer *, uint32_t serial, uint32_t,
                   uint32_t button, uint32_t state) {
  if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED &&
    XDG_TOPLEVEL_RESIZE_EDGE_NONE != window_resize_edge)
    xdg_toplevel_resize(window, seat, serial, window_resize_edge);
}

void pointer_frame(void *, wl_pointer *) {
  if (cursor_image == nullptr)
    return;

  wl_buffer *cursor_buffer = wl_cursor_image_get_buffer(cursor_image);
  wl_surface_damage(cursor_surface, 0, 0, cursor_image->width,
                    cursor_image->height);
  wl_surface_attach(cursor_surface, cursor_buffer, 0, 0);
  wl_surface_commit(cursor_surface);
}

wl_pointer_listener pointer_listener = {.enter = pointer_enter,
                                        .leave = pointer_leave,
                                        .motion = pointer_motion,
                                        .button = pointer_click,
                                        .frame = pointer_frame};

void lowlib_set_cursor(const char *(*cursor)(int x, int y)) {
  set_xcursor = cursor;
}

void lowlib_set_border(uint8_t custom_border) { border = custom_border; }

const char *fallback_set_xcursor(int x, int y) { return XCURSOR_LEFT_POINTER; }

void render_cursor(xdg_toplevel*window_now, uint16_t x_max_now, uint16_t y_max_now) {
  x_max = x_max_now;
  y_max = y_max_now;
  window = window_now;
}

const char *set_xcursor_resize(int x, int y) {

  if (x <= border && y <= border) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_LEFT;
    return "nwse-resize";
  }
  if (x + border >x_max && y + border >y_max) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT;
    return "nwse-resize";
  }
  if (x <= border && y + border >y_max) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_LEFT;
    return "nesw-resize";
  }
  if (x + border >x_max && y <= border) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP_RIGHT;
    return "nesw-resize";
  }
  if (x <= border) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
    return XCURSOR_HORIZONTAL_RESIZE;
  }
  if (x + border >x_max) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
    return XCURSOR_HORIZONTAL_RESIZE;
  }
  if (y <= border) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_TOP;
    return XCURSOR_VERTICAL_RESIZE;
  }
  if (y + border >y_max) {
    window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
    return XCURSOR_VERTICAL_RESIZE;
  }
  window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_NONE;
  return XCURSOR_LEFT_POINTER;
}
