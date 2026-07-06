#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <string.h>
#include "xdg-protocol.h"

#define WAYLAND_DISPLAY NULL

struct interface_handles {
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_seat *seat;
	struct xdg_wm_base *wm;
	struct wl_surface *z_index;
	struct xdg_toplevel *window;
	struct xdg_surface *surface;
	int32_t width, height;
	uint8_t running;
	struct wl_keyboard *keyboard;
	struct wl_pointer *pointer;
};

// Controling form background redraw
static struct wl_buffer *frame_buffer(struct wl_shm *shm, int32_t width, int32_t height)
{
    int32_t offset = 0, stride = width << 2, pixmap_size = stride * height;

    int32_t fd = syscall(SYS_memfd_create, "buffer", 0);
    ftruncate(fd, pixmap_size);
    uint32_t *pixels = mmap(NULL, pixmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Draw pixels on canvas
    pixels[0] = 0x00FFFF99;
    memcpy(pixels + 0x01, pixels, sizeof(uint32_t));
    memcpy(pixels + 0x10, pixels, 0x10 * sizeof(uint32_t));
    uint8_t chunk_size = 4;
    for (uint32_t i = chunk_size >> 1; i < (pixmap_size >> 2); i += chunk_size) {
        memcpy(pixels + i, pixels, chunk_size * sizeof(uint32_t));
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, pixmap_size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, offset,
                width, height, stride, WL_SHM_FORMAT_XRGB8888);
    close(fd);
    return buffer;
}

static void
xdg_surface_configure(void *data, struct xdg_surface *form, uint32_t serial)
{
    struct interface_handles *handles = data;
	xdg_surface_ack_configure(form, serial);
	int32_t width = handles->width ? handles->width : 800, height = handles->height ? handles->height : 450;
	wl_surface_damage(handles->z_index, 0, 0, width, height);
	wl_surface_attach(handles->z_index, frame_buffer(handles->shm, width, height), 0, 0);
	wl_surface_commit(handles->z_index);
}

static const struct xdg_surface_listener surface_listener = {
	.configure = xdg_surface_configure
};

// Configure window
static void
configure_window(void *data,
    struct xdg_toplevel *xdg_toplevel,
    int32_t width, int32_t height,
    struct wl_array *states)
{
    struct interface_handles *handles = data;
    handles->width = width; handles->height = height;
}

static void
close_window(void *data, struct xdg_toplevel *form)
{
    struct interface_handles *handles = data;
    handles->running = 0;
}

static void
manage_window(void *data,
    struct xdg_toplevel *form,
    int32_t width, int32_t height)
{
    struct interface_handles *handles = data;
    handles->width = width; handles->height = height;
}

static void
window_capabilities(void *data,
    struct xdg_toplevel *xdg_toplevel,
    struct wl_array *capabilities) { /* NOOP */ }

static const struct xdg_toplevel_listener window_listener = {
    .configure = configure_window,
    .close = close_window,
    .configure_bounds = manage_window,
    .wm_capabilities = window_capabilities
};

// Configuring user input devices
static void
wl_seat_capabilities(void *data, struct wl_seat *seat,
	uint32_t capabilities)
{
	struct interface_handles *handles = data;

	if(capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
	{
		fprintf(stderr, "Keyboard detected\n");
		handles->keyboard = wl_seat_get_keyboard(seat);
	}

	if(capabilities & WL_SEAT_CAPABILITY_POINTER)
	{
		fprintf(stderr, "Mouse or TouchPad detected\n");
		handles->pointer = wl_seat_get_pointer(seat);
	}

	if(capabilities & WL_SEAT_CAPABILITY_TOUCH)
	{
		fprintf(stderr, "Touchscreen or graphics tablet detected\n");
	}
}

static void
wl_seat_name(void *data, struct wl_seat *seat,
	const char *name)
{
	fprintf(stderr, "User input device: %s\n", name);
}

static const struct wl_seat_listener user_input_listener = {
	.capabilities = wl_seat_capabilities,
	.name = wl_seat_name
};

// Configuring window decoration and window event messages
static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *window_manager,
	uint32_t serial)
{
	xdg_wm_base_pong(window_manager, serial);
}

static const struct xdg_wm_base_listener xdg_listener = {
	.ping = xdg_wm_base_ping
};

// Registry listeners to inquiry feature interfaces
static void
registry_handle_global(void *data, struct wl_registry *registry,
	uint32_t name, const char *interface, uint32_t version)
{
	struct interface_handles *handles = data;

	// Wayland compositor interface
	if (strcmp(interface, wl_compositor_interface.name) == 0)
	{
		handles->compositor = wl_registry_bind(registry,
			name, &wl_compositor_interface, version);
		handles->z_index = wl_compositor_create_surface(handles->compositor);
	}

	// Wayland shared memory interface for UI buffer
	if (strcmp(interface, wl_shm_interface.name) == 0)
		handles->shm = wl_registry_bind(registry,
			name, &wl_shm_interface, version);

	// Wayland input interface for mouse, keyboard and touch
	if (strcmp(interface, wl_seat_interface.name) == 0)
	{
		handles->seat = wl_registry_bind(registry,
			name, &wl_seat_interface, version);
		wl_seat_add_listener(handles->seat, &user_input_listener, handles);
	}

	// Window Manager interface for window decoration and window resize message handling
	if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		handles->wm = wl_registry_bind(registry,
			name, &xdg_wm_base_interface, version);
		xdg_wm_base_add_listener(handles->wm, &xdg_listener, handles);
		handles->surface = xdg_wm_base_get_xdg_surface(handles->wm, handles->z_index);
		handles->window = xdg_surface_get_toplevel(handles->surface);
		xdg_toplevel_set_title(handles->window, "Wayland Form");
		xdg_toplevel_set_app_id(handles->window, "wlform");

		xdg_toplevel_add_listener(handles->window, &window_listener, handles);
		xdg_surface_add_listener(handles->surface, &surface_listener, handles);

		wl_surface_commit(handles->z_index);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
	uint32_t name) { /* NOOP */ }

static const struct wl_registry_listener registry_listener = {
	.global = registry_handle_global,
	.global_remove = registry_handle_global_remove
};

int main()
{
	// Handles of Wayland interfaces and objects
	struct interface_handles handles = { 0 };
	handles.width = 800; handles.height = 450;
	// Connect display
	struct wl_display *display = wl_display_connect(WAYLAND_DISPLAY);
	if (!display)
	{
		fprintf(stderr, "Failed to connect to Wayland display\n");
		return -1;
	} else fprintf(stderr, "Wayland compositor connected\n");

	// Query registry for available wayland features
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &handles);
	wl_display_roundtrip(display);
	wl_surface_commit(handles.z_index);

	// Keeping non-ui process alive
	handles.running = UINT8_MAX;
	while (wl_display_dispatch(display) != -1) {
	    if (!handles.running) break;
	}

	// Closing display connection
	wl_surface_destroy(handles.z_index);
	wl_registry_destroy(registry);
	wl_display_disconnect(display);
	return 0;
}
