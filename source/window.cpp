#include "window.hpp"
#include "lowlib.hpp"
#include "registry.hpp"
#include "renderer.hpp"
#include <cstdint>
#include <wayland-util.hpp>

bool resizable_window = true;
int window_initial_width = 480, window_initial_height = 320;
char window_resize_state = LOWLIB_WINDOW_NORMAL;

void lowlib_set_resize(bool resizable) { resizable_window = resizable; }
void lowlib_set_window(int width, int height) {
  if (window_initial_width < width)
    window_initial_width = width;
  if (window_initial_height < height)
    window_initial_height = height;
}
char lowlib_get_window() { return window_resize_state; }
void xdg_wm_base_ping(void *, xdg_wm_base *wm, uint32_t serial) {
  xdg_wm_base_pong(wm, serial);
}
xdg_wm_base_listener wm_listener = {.ping = xdg_wm_base_ping};

void xdg_surface_configure(void * data, xdg_surface *surface, uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);
  auto app_state_now = static_cast<app_state*>(data);
  app_state_now->running = true;
}
xdg_surface_listener surface_listener = {.configure = xdg_surface_configure};

void close_window(void *data, xdg_toplevel *) {
  auto current_app_state = static_cast<app_state*>(data);
  current_app_state->running = 0;
}
void resize_window(void *, xdg_toplevel *, int32_t, int32_t, wl_array *);
void scale_window(void *, xdg_toplevel *, int32_t, int32_t);
void window_caps(void*,xdg_toplevel*,wl_array*);
xdg_toplevel_listener window_listener = {.configure = resize_window,
                                         .close = close_window,
                                         .configure_bounds = scale_window,
                                         .wm_capabilities = window_caps
};

void scale_window(void *, xdg_toplevel *, int32_t screen_width,
                  int32_t screen_height) {
  int32_t shorter_length =
      screen_height < screen_width ? screen_height : screen_width;
  if (shorter_length > 1080 and !resizable_window) {
    window_initial_width = shorter_length;
    window_initial_height = shorter_length << 1;
  }
}

void resize_window(void *, xdg_toplevel *window, int32_t width, int32_t height,
                   wl_array *states) {
  int *state;
  char window_resize_state_impl = LOWLIB_WINDOW_NORMAL;
  wl_array_for_each_cpp(state, states) {
    if (XDG_TOPLEVEL_STATE_FULLSCREEN == *state) {
      if (resizable_window) {
        window_resize_state_impl = LOWLIB_WINDOW_FULLSCREEN;
      } else {
        xdg_toplevel_unset_fullscreen(window);
        return;
      }
    }
    if (XDG_TOPLEVEL_STATE_MAXIMIZED == *state && resizable_window) {
      if (resizable_window) {
        window_resize_state_impl = LOWLIB_WINDOW_MAXIMIZED;
      } else {
        xdg_toplevel_unset_maximized(window);
        return;
      }
    }
  }

  window_resize_state = window_resize_state_impl;
  if (!resizable_window && LOWLIB_WINDOW_NORMAL == window_resize_state_impl) {
    xdg_toplevel_set_max_size(window, window_initial_width,
                              window_initial_height);
    width = window_initial_width;
    height = window_initial_height;
  }
  create_frame_buffer(width, height);
}
void window_caps(void*,xdg_toplevel*,wl_array*states) {
  int*state;
  wl_array_for_each_cpp(state, states) {
    if (XDG_TOPLEVEL_STATE_ACTIVATED == *state)
    {
      // TODO Explore capabilities
    }
  }
}
