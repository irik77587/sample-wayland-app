#include "interface.hpp"
#include <cstddef>
#include <cstdlib>
#include <wayland-cursor.h>

int main() {
  const char *display_name = getenv("WAYLAND_DISPLAY");
  if (display_name == nullptr) {
    display_name = NULL;
  }
  // Initialize
  #ifdef DEBUG
  fprintf(stderr, "Connecting to display server: %s\n", display_name);
  #endif
  wl_display *display = wl_display_connect(display_name);
  wl_registry *registry = wl_display_get_registry(display);
#ifdef DEBUG
  fprintf(stderr, "display and registry initialized\n");
#endif

  // Configure constructor
  struct handles m_handles = {0};
  wl_registry_add_listener(registry, &listen_registry, &m_handles);

  // Finalize constructor
  wl_display_roundtrip(display);
  m_handles.running = 1;
#ifdef DEBUG
  fprintf(stderr, "feature registry complete\n");
#endif

  // mainloop
  do {
#ifdef DEBUG
    fprintf(stderr, "dispatching\n");
#endif
    wl_display_dispatch(display);
  } while (m_handles.running);

  wl_display_roundtrip(display);

  // destructor
  if (m_handles.cursor_surface) {
    wl_surface_destroy(m_handles.cursor_surface);
  }

  if (m_handles.canvas) {
    wl_surface_destroy(m_handles.canvas);
  }

  wl_display_roundtrip(display);
  wl_registry_destroy(registry);
  wl_display_disconnect(display);
  return 0;
}
