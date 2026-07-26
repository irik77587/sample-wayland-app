#include "renderer.hpp"
#include "shm.hpp"
#include "lowlib.hpp"
#include <cstdint>
#include <unistd.h>
#include <wayland-client.h>
#include <syscall.h>
#include <sys/mman.h>

lowlib_color fallback_shade_pixels(uint16_t,uint16_t,uint16_t,uint16_t);

lowlib_color (*shade_pixels)(uint16_t,uint16_t,uint16_t,uint16_t) = fallback_shade_pixels;

void lowlib_set_shader(lowlib_color(*shader)(unsigned short, unsigned short, unsigned short, unsigned short)) { shade_pixels = shader; }

static lowlib_color color_back = {0x8c, 0xb8, 0x3f}, color_fore = {0xac, 0xd8, 0x5f};
lowlib_color fallback_shade_pixels(uint16_t x,uint16_t y,uint16_t x_max,uint16_t y_max)
{
    return color_back;
}

void release_buffer(void*,wl_buffer*buffer) { wl_buffer_destroy(buffer); }

wl_buffer_listener buffer_listener = {
    .release = release_buffer
};

void create_frame_buffer(unsigned int width,unsigned int height)
{
    wl_surface_damage(window_surface, 0, 0, width, height);
    uint8_t color_depth = shm_format == WL_SHM_FORMAT_XRGB8888 ? 4 : 3;
    uint16_t x_max = width, y_max = height;
    uint32_t stride = color_depth *width, pixels_count = width*height,
    buffer_size = color_depth *pixels_count, pos=0, fd = syscall(SYS_memfd_create, "lowlib", 0);
    ftruncate(fd,buffer_size);
    auto*pixels=static_cast<uint8_t*>(mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    wl_shm_pool*pool=wl_shm_create_pool(shm, fd, buffer_size);
    wl_buffer*buffer=wl_shm_pool_create_buffer(pool, 0, width, height, stride, shm_format);
    switch (shm_format) {
        case WL_SHM_FORMAT_XRGB8888: {
            for (uint16_t y = 0; y < height; y++)
                for (uint16_t x = 0; x < width; x++) {
                    lowlib_color color = shade_pixels(x, y, x_max, y_max);
                    pixels[pos++] = 0;
                    pixels[pos++] = color.r;
                    pixels[pos++] = color.g;
                    pixels[pos++] = color.b;
                }
            break;
        }
        case WL_SHM_FORMAT_RGB888: {
            for (uint16_t y = 0; y < height; y++)
                for (uint16_t x = 0; x < width; x++) {
                    lowlib_color color = shade_pixels(x, y, x_max, y_max);
                    pixels[pos++] = color.r;
                    pixels[pos++] = color.g;
                    pixels[pos++] = color.b;
                }
            break;
        }
        case WL_SHM_FORMAT_BGR888: {
            for (uint16_t y = 0; y < height; y++)
                for (uint16_t x = 0; x < width; x++) {
                    lowlib_color color = shade_pixels(x, y, x_max, y_max);
                    pixels[pos++] = color.b;
                    pixels[pos++] = color.g;
                    pixels[pos++] = color.r;
                }
            break;
        }
    }
    wl_shm_pool_destroy(pool);
    close(fd);
    wl_buffer_add_listener(buffer, &buffer_listener, NULL);
    wl_surface_attach(window_surface, buffer, 0, 0);
    wl_surface_commit(window_surface);
}
