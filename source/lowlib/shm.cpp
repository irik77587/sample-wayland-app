#include "shm.hpp"
#include <cstdint>
#include <wayland-client.h>

void get_formats(void *, wl_shm *, uint32_t);

uint32_t shm_format = WL_SHM_FORMAT_XRGB8888;

wl_shm_listener shm_listener = {.format = get_formats};

void get_formats(void *, wl_shm *, uint32_t format) {
  if (WL_SHM_FORMAT_RGB888 == format) {
    shm_format = format;
  }
  if (WL_SHM_FORMAT_XRGB8888 == shm_format && WL_SHM_FORMAT_BGR888 == format) {
    shm_format = format;
  }
}
