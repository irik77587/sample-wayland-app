#include "interface.hpp"
#include <wayland-client-protocol.h>
#include <wayland-util.h>

void xdg_surface_configure(void *, struct xdg_surface *surface,
                           uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);
}

void window_close(void *data, struct xdg_toplevel *) {
  auto m_handles = static_cast<handles *>(data);
  m_handles->running = 0;
}
struct wl_buffer *draw(struct wl_shm *, uint16_t, uint16_t);

void window_reconfigure(void *data, struct xdg_toplevel *window, int32_t,
                        int32_t, struct wl_array *states) {
  auto m_handles = static_cast<handles *>(data);
  wl_surface_damage(m_handles->canvas, 0, 0, 480, 320);
  for (auto state = static_cast<uint32_t *>((states)->data);
       (states)->size != 0 &&
       reinterpret_cast<const char *>(state) <
           (static_cast<const char *>((states)->data) + (states)->size);
       (state)++) {
    if (XDG_TOPLEVEL_STATE_MAXIMIZED == *state) {
      xdg_toplevel_unset_fullscreen(window);
      return;
    }
    if (XDG_TOPLEVEL_STATE_FULLSCREEN == *state) {
      xdg_toplevel_unset_maximized(window);
      return;
    }
  }
  xdg_toplevel_set_max_size(window, 480, 320);
  wl_buffer *buffer = renderframe(m_handles->shm, 480, 320);
  wl_surface_attach(m_handles->canvas, buffer, 0, 0);
  wl_surface_commit(m_handles->canvas);
  wl_buffer_add_listener(buffer, &listen_buffer, NULL);
}
void window_preconfigure(void *, struct xdg_toplevel *, int32_t, int32_t) {}
void window_caps(void *, struct xdg_toplevel *, struct wl_array *) {}
