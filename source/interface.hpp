#pragma once
#include "xdg-protocol.h"
#include <cstdint>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>
#include <wayland-util.h>

#ifdef DEBUG
#include <cstdio>
#endif

struct handles {
  wl_surface *canvas;
  wl_shm *shm;
  wl_surface *cursor_surface;
  wl_cursor_theme *cursor_theme;
  wl_cursor_image *cursor_image;
  int8_t running;
};

extern struct wl_registry_listener listen_registry;

void features_enable(void *, struct wl_registry *, uint32_t, const char *,
                     uint32_t);
void features_disable(void *, struct wl_registry *, uint32_t);

extern struct wl_seat_listener listen_event;

void event_types(void *, struct wl_seat *, uint32_t);
void event_group(void *, struct wl_seat *, const char *);

extern struct wl_pointer_listener listen_event_pointer;

void event_pointer_enter(void *, struct wl_pointer *, uint32_t,
                         struct wl_surface *, wl_fixed_t, wl_fixed_t);
void event_pointer_leave(void *, struct wl_pointer *, uint32_t,
                         struct wl_surface *);
void event_pointer_hover(void *, struct wl_pointer *, uint32_t, wl_fixed_t,
                         wl_fixed_t);
void event_pointer_click(void *, struct wl_pointer *, uint32_t, uint32_t,
                         uint32_t, uint32_t);
void event_pointer_frame(void *, struct wl_pointer *);

extern struct xdg_wm_base_listener listen_wm;
extern struct xdg_surface_listener lister_surface;
extern struct xdg_toplevel_listener listen_window;

void xdg_surface_configure(void *, struct xdg_surface *, uint32_t);

void window_close(void *, struct xdg_toplevel *);
void window_reconfigure(void *, struct xdg_toplevel *, int32_t, int32_t,
                        struct wl_array *);
void window_preconfigure(void *, struct xdg_toplevel *, int32_t, int32_t);
void window_caps(void *, struct xdg_toplevel *, struct wl_array *);

extern struct wl_buffer_listener listen_buffer;
void buffer_release(void*,struct wl_buffer*);
wl_buffer * renderframe(wl_shm *shm, uint16_t length, uint16_t height);

extern struct wl_shm_listener listen_formats;
void supported_formats(void*,struct wl_shm*, uint32_t);
