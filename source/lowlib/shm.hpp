#pragma once
#include <cstdint>
#include <wayland-client.h>

extern wl_shm_listener shm_listener;
extern uint32_t shm_format;
extern wl_shm*shm;
extern wl_surface*window_surface,*cursor_surface;
