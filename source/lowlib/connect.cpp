#include "../lowlib.hpp"
#include "registry.hpp"
#include <cstdio>
#include <cstdlib>
#include <wayland-client.h>
#include "cursor-theme.hpp"

int main() {
  app_state app = {0};
  app.cursor_theme_name = get_cursor_theme();
  char *display_server = getenv("WAYLAND_DISPLAY");
  wl_display *display = wl_display_connect(display_server);
  if (!display_server) {
#ifdef DEVBUILD
    printf("ERROR : Couldn't connect to display server: %s\n", display_server);
#endif
    return EXIT_FAILURE;
  }

  wl_registry *registry = wl_display_get_registry(display);
  if (!registry) {
#ifdef DEVBUILD
    printf("ERROR : Couldn't connect to display server: %s\n", display_server);
#endif
    return EXIT_FAILURE;
  }
  wl_registry_add_listener(registry, &registry_listener, &app);
  wl_display_roundtrip(display);
  init_res(&app);
  lowlib_set_bootup();
  do {
    wl_display_dispatch(display);
  } while (app.running);
  wl_display_roundtrip(display);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);
}
