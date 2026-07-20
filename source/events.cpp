#include "interface.hpp"
#include <wayland-client-protocol.h>
#include <wayland-cursor.h>

#define CURSOR_THEME "breeze_cursors"

void event_group(void *, wl_seat *, const char *) {}
void event_types(void *data, wl_seat *seat, uint32_t caps) {
  auto p_handles = static_cast<handles *>(data);
  if (caps & WL_SEAT_CAPABILITY_POINTER) {
    wl_pointer *pointer = wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &listen_event_pointer, p_handles);
#ifdef DEBUG
    printf("pointer event listener added\n");
#endif
  }
}

void event_pointer_enter(void *data, struct wl_pointer *pointer,
                         uint32_t serial, struct wl_surface *, wl_fixed_t x,
                         wl_fixed_t y) {
  auto p_handles = static_cast<handles *>(data);
  // Start cursor loading
  p_handles->cursor_theme =
      wl_cursor_theme_load(CURSOR_THEME, 24, p_handles->shm);
  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(p_handles->cursor_theme, "left_ptr");
  p_handles->cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, p_handles->cursor_surface,
                        p_handles->cursor_image->hotspot_x,
                        p_handles->cursor_image->hotspot_y);
}

void event_pointer_leave(void *data, struct wl_pointer *, uint32_t,
                         struct wl_surface *) {
  auto p_handles = static_cast<handles *>(data);
  if (p_handles->cursor_theme == nullptr)
    return;
  wl_cursor_theme_destroy(p_handles->cursor_theme);
  p_handles->cursor_image = nullptr;
  p_handles->cursor_theme = nullptr;
}

void event_pointer_hover(void *data, struct wl_pointer *pointer,
                         uint32_t serial, wl_fixed_t, wl_fixed_t) {
  auto p_handles = static_cast<handles *>(data);
  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(p_handles->cursor_theme, "left_ptr");
  p_handles->cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, p_handles->cursor_surface,
                        p_handles->cursor_image->hotspot_x,
                        p_handles->cursor_image->hotspot_y);
  // Done cursor loading
}

void event_pointer_click(void *, struct wl_pointer *, uint32_t, uint32_t,
                         uint32_t, uint32_t) {}

void event_pointer_frame(void *data, struct wl_pointer *) {
  auto p_handles = static_cast<handles *>(data);
  if (p_handles->cursor_image == nullptr)
    return;

  struct wl_buffer *cursor_buffer =
      wl_cursor_image_get_buffer(p_handles->cursor_image);
  wl_surface_damage(p_handles->cursor_surface, 0, 0,
                    p_handles->cursor_image->width,
                    p_handles->cursor_image->height);
  wl_surface_attach(p_handles->cursor_surface, cursor_buffer, 0, 0);
  wl_surface_commit(p_handles->cursor_surface);
}
