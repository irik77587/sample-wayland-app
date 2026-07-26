#include "cursor.hpp"
#include "registry.hpp"
#include "shm.hpp"
#include <wayland-cursor.h>
#include <wayland-util.h>
#include "../lowlib.hpp"

const char*fallback_set_xcursor(int x, int y);
const char*(*set_xcursor)(int x,int y) = fallback_set_xcursor;

wl_cursor_theme *cursor_theme;
wl_cursor_image *cursor_image;

void pointer_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
                   struct wl_surface *, wl_fixed_t u, wl_fixed_t v) {
  auto app = static_cast<app_state *>(data);
  int x = wl_fixed_to_int(u), y = wl_fixed_to_int(v);
  // Start cursor loading
  const char*cursor_type = set_xcursor(x, y);
  cursor_theme = wl_cursor_theme_load(app->cursor_theme_name, 24, shm);
  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(cursor_theme, cursor_type);
  cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, cursor_surface,
                        cursor_image->hotspot_x, cursor_image->hotspot_y);
}

void pointer_leave(void *, struct wl_pointer *, uint32_t, struct wl_surface *) {
  if (cursor_theme == nullptr)
    return;
  wl_cursor_theme_destroy(cursor_theme);
  cursor_image = nullptr;
  cursor_theme = nullptr;
}

void pointer_motion(void *data, struct wl_pointer *pointer, uint32_t serial,
                    wl_fixed_t u, wl_fixed_t v) {
  int x = wl_fixed_to_int(u), y = wl_fixed_to_int(v);
  const char*cursor_type = set_xcursor(x, y);
  struct wl_cursor *cursor =
      wl_cursor_theme_get_cursor(cursor_theme, cursor_type);
  cursor_image = cursor->images[0];
  wl_pointer_set_cursor(pointer, serial, cursor_surface,
                        cursor_image->hotspot_x, cursor_image->hotspot_y);
  // Done cursor loading
}

void pointer_click(void *, wl_pointer *, uint32_t, uint32_t, uint32_t,
                   uint32_t) {}

void pointer_frame(void *, wl_pointer *) {
  if (cursor_image == nullptr)
    return;

  wl_buffer *cursor_buffer = wl_cursor_image_get_buffer(cursor_image);
  wl_surface_damage(cursor_surface, 0, 0, cursor_image->width,
                    cursor_image->height);
  wl_surface_attach(cursor_surface, cursor_buffer, 0, 0);
  wl_surface_commit(cursor_surface);
}


wl_pointer_listener pointer_listener = {
  .enter = pointer_enter,
  .leave = pointer_leave,
  .motion = pointer_motion,
  .button = pointer_click,
  .frame = pointer_frame
};

void lowlib_set_cursor(const char*(*cursor)(int x, int y))
{
  set_xcursor = cursor;
}
const char*fallback_set_xcursor(int x, int y)
{
  return XCURSOR_LEFT_POINTER;
}
