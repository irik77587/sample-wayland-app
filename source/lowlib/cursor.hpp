#pragma once
#include "inc/xdg-shell.h"
#include <cstdint>
#include <wayland-client.h>

extern wl_pointer_listener pointer_listener;

void render_cursor(xdg_toplevel*window, uint16_t width,uint16_t height);
