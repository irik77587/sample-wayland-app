#include "interface.hpp"
#include <cstring>

void features_disable(void *, wl_registry *, uint32_t) {}
void features_enable(void *data, wl_registry *registry, uint32_t name,
                     const char *interface, uint32_t version) {
  auto p_handles = static_cast<handles*>(data);

  if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
    auto compositor = static_cast<wl_compositor*>(wl_registry_bind(
        registry, name, &wl_compositor_interface, version));
    p_handles->canvas = wl_compositor_create_surface(compositor);
    p_handles->cursor_surface = wl_compositor_create_surface(compositor);
    return;
  }

  if (std::strcmp(interface, wl_shm_interface.name) == 0) {
    p_handles->shm =
        static_cast<wl_shm *>(wl_registry_bind(registry, name, &wl_shm_interface, version));
        wl_shm_add_listener(p_handles->shm, &listen_formats, NULL);
        return;
  }

  if (std::strcmp(interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat = static_cast<wl_seat*>(wl_registry_bind(
        registry, name, &wl_seat_interface, version));
    wl_seat_add_listener(seat, &listen_event, p_handles);
    return;
  }

  if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
    struct xdg_wm_base *wm = static_cast<xdg_wm_base*>(wl_registry_bind(
        registry, name, &xdg_wm_base_interface, version));
    xdg_wm_base_add_listener(wm, &listen_wm, p_handles);
    struct xdg_surface *surface =
        xdg_wm_base_get_xdg_surface(wm, p_handles->canvas);
    struct xdg_toplevel *window = xdg_surface_get_toplevel(surface);
    xdg_surface_add_listener(surface, &lister_surface, p_handles);
    xdg_toplevel_add_listener(window, &listen_window, p_handles);
    wl_surface_commit(p_handles->canvas);
    return;
  }

  return;
}
