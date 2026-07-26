#include "event.hpp"
#include "lowlib.hpp"
#include <cstdint>
#include <cstring>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include "cursor.hpp"
#include "registry.hpp"

char *set_xcursor_default(int x, int y);

char *(*set_xcursor_name)(int x, int y) = set_xcursor_default;

void lowlib_set_cursor(char *(cb)(int x, int y)) { set_xcursor_name = cb; }

void seat_caps(void *, wl_seat *, uint32_t);
void seat_name(void *, wl_seat *, const char *);

wl_seat_listener seat_listener = {.capabilities = seat_caps, .name = seat_name};

void seat_name(void *, wl_seat *, const char *) {}

void seat_caps(void *data, wl_seat *seat, uint32_t caps) {
  auto app = static_cast<app_state*>(data);
  if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {  }
  if (caps & WL_SEAT_CAPABILITY_POINTER) {
    wl_pointer*pointer=wl_seat_get_pointer(seat);
    wl_pointer_add_listener(pointer, &pointer_listener, app);
  }
  if (caps & WL_SEAT_CAPABILITY_TOUCH) {  }
}

char *set_xcursor_default(int x, int y) {
  char *xcursor_name;
  strcpy(xcursor_name, "left-ptr");
  return xcursor_name;
}

