#pragma once
#include <wayland-client.h>
extern wl_registry_listener registry_listener;

struct app_state {
    bool running;
    const char*cursor_theme_name;
};

void init_res(app_state*);
