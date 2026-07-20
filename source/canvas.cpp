#include "interface.hpp"
#include <cstdint>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

uint32_t render_format = WL_SHM_FORMAT_XRGB8888;

void supported_formats(void *data, struct wl_shm *, uint32_t format) {
  handles *m_handles = static_cast<handles *>(data);
#ifdef DEBUG
  fprintf(stderr, "shm format: %x\n", format);
#endif
  if (render_format == WL_SHM_FORMAT_RGB888) {
    return;
  }
  if (format == WL_SHM_FORMAT_RGB888 || format == WL_SHM_FORMAT_BGR888) {
    render_format = format;
  }
}

void buffer_release(void *, struct wl_buffer *buffer) {
  wl_buffer_destroy(buffer);
}

struct color_rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

color_rgb shaderlogic(uint16_t x, uint16_t y, uint16_t x_max, uint16_t y_max) {
  color_rgb color_back = {0x8c, 0xb8, 0x3f}, color_fore = {0xac, 0xd8, 0x5f},
            color_form = {0xff, 0xff, 0xff};
  uint8_t pad = 2;
  if (x < pad || y < pad || x + pad > x_max || y + pad > y_max)
    return color_back;
  pad += 8;
  if (x < pad || y < pad || x + pad > x_max || y + pad > y_max)
    return color_fore;
  pad += 2;
  if (x < pad || y < pad || x + pad > x_max || y + pad > y_max)
    return color_back;
  return color_form;
}

struct wl_buffer *renderframe(wl_shm *shm, uint16_t length, uint16_t height) {
  uint8_t color_depth = render_format == WL_SHM_FORMAT_XRGB8888
                            ? 4
                            : 3; // 3 bytes RGB or BGR color
  uint32_t stride = color_depth * length, pixel_count = length * height,
           buffer_size = color_depth * pixel_count, pos = 0;
  int32_t fd = syscall(SYS_memfd_create, "i735m", 0);
  ftruncate(fd, buffer_size);
  auto *data = static_cast<uint8_t *>(
      mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  wl_shm_pool *pool = wl_shm_create_pool(shm, fd, buffer_size);
  wl_buffer
      *buffer =
          wl_shm_pool_create_buffer(pool,
                                    0, length, height, stride, render_format /* e.g. WL_SHM_FORMAT_XRGB8888 or if available WL_SHM_FORMAT_RGB888 */);
  uint16_t x_max = length - 1, y_max = height - 1;
  switch (render_format) {
  case WL_SHM_FORMAT_XRGB8888: {
    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < length; x++) {
        color_rgb color = shaderlogic(x, y, x_max, y_max);
        data[pos++] = 0; // empty alpha channel;
        data[pos++] = color.r;
        data[pos++] = color.g;
        data[pos++] = color.b;
      }
    }
        break;
  }
  case WL_SHM_FORMAT_RGB888: {
    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < length; x++) {
        color_rgb color = shaderlogic(x, y, x_max, y_max);
        data[pos++] = color.r;
        data[pos++] = color.g;
        data[pos++] = color.b;
      }
    }
        break;
  }
  case WL_SHM_FORMAT_BGR888: {
    for (uint16_t y = 0; y < height; y++) {
      for (uint16_t x = 0; x < length; x++) {
        color_rgb color = shaderlogic(x, y, x_max, y_max);
        data[pos++] = color.b;
        data[pos++] = color.g;
        data[pos++] = color.r;
      }
    }
        break;
  }
  }
  wl_shm_pool_destroy(pool);
  close(fd);
  return buffer;
}
