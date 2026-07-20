#include "interface.hpp"
#include <wayland-client-protocol.h>

struct wl_registry_listener listen_registry = {
    .global = features_enable, .global_remove = features_disable};

struct wl_seat_listener listen_event = {.capabilities = event_types,
                                        .name = event_group};

struct wl_pointer_listener listen_event_pointer = {
    .enter = event_pointer_enter,
    .leave = event_pointer_leave,
    .motion = event_pointer_hover,
    .button = event_pointer_click,
    .frame = event_pointer_frame};

void wm_ping(void *, struct xdg_wm_base *wm, uint32_t serial) {
  xdg_wm_base_pong(wm, serial);
}

struct xdg_wm_base_listener listen_wm = {.ping = wm_ping};

struct xdg_surface_listener lister_surface = {.configure =
                                                  xdg_surface_configure};
struct xdg_toplevel_listener listen_window = {.configure = window_reconfigure,
                                              .close = window_close,
                                              .configure_bounds =
                                                  window_preconfigure,
                                              .wm_capabilities = window_caps};

struct wl_buffer_listener listen_buffer = {
    .release = buffer_release
};

struct wl_shm_listener listen_formats = {
    .format = supported_formats
};

