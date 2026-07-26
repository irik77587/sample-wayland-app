#include "registry.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <cstdio>
#include "inc/xdg-shell.h"
#include "event.hpp"
#include "shm.hpp"
#include "window.hpp"

void global(void*,wl_registry*,uint32_t,const char*,uint32_t);
void global_remove(void*,wl_registry*,uint32_t);
uint32_t feature_names[4];
// Can't define inside ->global(...)<- function
// Need global scope
wl_compositor*composer;wl_shm*shm;xdg_wm_base*wm;
wl_surface*window_surface,*cursor_surface;

wl_registry_listener registry_listener = {
    .global = global,
    .global_remove = global_remove
};

void global_remove(void*,wl_registry*,uint32_t name)
{
    if (feature_names[1] == name){
        shm = NULL;
        feature_names[1] = 0;
    }
}

void global(void*data, wl_registry*registry, uint32_t name, const char*interface, uint32_t version)
{
    auto m_app = static_cast<app_state*>(data);
    if (strcmp(interface, wl_compositor_interface.name) == 0 && version >= 6)
    {
        composer = static_cast<wl_compositor*>(wl_registry_bind(registry,name,&wl_compositor_interface,version));
        feature_names[0] = name;
    }
    if (strcmp(interface, wl_shm_interface.name) == 0 && version >= 1)
    {
        shm = static_cast<wl_shm*>(wl_registry_bind(registry,name,&wl_shm_interface,version));
        wl_shm_add_listener(shm,&shm_listener,NULL);
        feature_names[1] = name;
    }
    if (strcmp(interface,wl_seat_interface.name) == 0 && version >= 9)
    {
        auto seat = static_cast<wl_seat*>(wl_registry_bind(registry,name,&wl_seat_interface,version));
        wl_seat_add_listener(seat,&seat_listener,m_app);
        feature_names[2] = name;
    }
    if (strcmp(interface,xdg_wm_base_interface.name) == 0 && version >= 6)
    {
        wm = static_cast<xdg_wm_base*>(wl_registry_bind(registry,name,&xdg_wm_base_interface,version));
        xdg_wm_base_add_listener(wm,&wm_listener,NULL);
        feature_names[3] = name;
    }
}

void init_res(app_state*current_app_state)
{
    window_surface=wl_compositor_create_surface(composer);
    cursor_surface=wl_compositor_create_surface(composer);

    xdg_surface*client_surface=xdg_wm_base_get_xdg_surface(wm, window_surface);
    xdg_surface_add_listener(client_surface,&surface_listener,current_app_state);
    xdg_toplevel*window=xdg_surface_get_toplevel(client_surface);
    xdg_toplevel_set_title(window, "lowlib window");
    xdg_toplevel_add_listener(window,&window_listener,current_app_state);
    wl_surface_commit(window_surface);
}
