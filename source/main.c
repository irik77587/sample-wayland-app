#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <string.h>
#include <wayland-util.h>
#include "xdg-protocol.h"
#include "cursor-shape.h"


/* Temporary solution remove unstable graphics tablet support to compile */
const struct wl_interface *zwp_tablet_tool_v2_interface = NULL;

#define WAYLAND_DISPLAY NULL
#define DEBUG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define MIN(a, b) \
	({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; \
	})

#define SD_SCREEN_LENGTH 480
#define SD_SCREEN_HEIGHT 360
#define SD_SCREEN_WIDTH SD_SCREEN_LENGTH

#define SVGA_SCREEN_LENGTH 640
#define SVGA_SCREEN_HEIGHT 480
#define SVGA_SCREEN_WIDTH SVGA_SCREEN_LENGTH

#define FHD_SCREEN_LENGTH 1920
#define FHD_SCREEN_HEIGTH 1080
#define FHD_SCREEN_WIDTH FHD_SCREEN_LENGTH

#define HD_SCREEN_WIDTH 1280
#define HD_SCREEN_HEIGHT 720

#define DEFAULT_TITLEBAR_HEIGHT 32
#define DEFAULT_BORDER_WIDTH 5

struct interface_handles {
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct wl_seat *seat;
	struct xdg_wm_base *wm;
	struct wl_surface *z_index;
	struct xdg_toplevel *window;
	struct xdg_surface *surface;
	uint16_t width, height, width_max, height_max;
	uint8_t border, running, fullscreen, maxscreen;
	uint32_t pointer_serial;
	struct wl_keyboard *keyboard;
	struct wl_pointer *pointer;
	// Server side cursor handler
	struct wp_cursor_shape_manager_v1 *cursor_shape_manager;
	struct wp_cursor_shape_device_v1 *cursor_shape_device;
	char* title;
	char app_id_8[8];
	int pointer_pos_x, pointer_pos_y;
};

// Controling form background redraw
static struct wl_buffer *frame_buffer(struct wl_shm *shm, int32_t width, int32_t height, uint8_t margin)
{
	int32_t offset = 0, stride = sizeof(uint32_t) * width, pixmap_len = width * height, pixmap_size = sizeof(uint32_t) * pixmap_len;

	int32_t fd = syscall(SYS_memfd_create, "buffer", 0);
	ftruncate(fd, pixmap_size);
	uint32_t *pixels = mmap(NULL, pixmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	// Draw pixels on canvas

	/* // Single color frameless window
	pixels[0] = 0x00FFFF99;
	for (
		uint32_t i = 1, chunk_size = i;
		i < pixmap_len;
		i = i << 1, chunk_size = MIN(i, pixmap_len - i) // compiler optimization may cause runtime error
	)
	{
		memcpy(pixels + i, pixels, chunk_size * sizeof(uint32_t));
	}
	*/

	// Multicolor window with frame CSD Client side decoration
	uint32_t border_color = 0x0077B900, button_color = 0x00FF2E12, window_color = 0x00FFFF99;
	for (uint16_t y = 0; y <= height; y++)
	{
		for (uint16_t x = 0; x <= width; x++)
		{
			uint32_t c = y * width + x, *color;
			if (margin && (x <= margin || x >= width - margin || y <= DEFAULT_TITLEBAR_HEIGHT || y >= height - margin)) {
				// Close window button
				if( y <= DEFAULT_TITLEBAR_HEIGHT - margin && x >= width - (DEFAULT_TITLEBAR_HEIGHT << 1) && x <= width - margin)
				{
					color = &button_color;
				}
				else
				{
					color = &border_color;
				}
			}
			else
			{
				color = &window_color;
			}
			memcpy(pixels + c, color, sizeof(uint32_t));
		}
	}

	struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, pixmap_size);
	struct wl_buffer *buffer = wl_shm_pool_create_buffer(
		pool, offset,
		width, height, stride,
		WL_SHM_FORMAT_XRGB8888
	);
	close(fd);
	wl_shm_pool_destroy(pool);
	return buffer;
}

static void
xdg_surface_configure(void *data, struct xdg_surface *form, uint32_t serial)
{
	struct interface_handles *handles = data;
	xdg_surface_ack_configure(form, serial);
	if(handles->width < SD_SCREEN_WIDTH) handles->width = SD_SCREEN_WIDTH;
	if(handles->height < SD_SCREEN_HEIGHT) handles->height = SD_SCREEN_HEIGHT;
	wl_surface_damage(handles->z_index, 0, 0, handles->width, handles->height);
	struct wl_buffer *buffer;
	if(handles->fullscreen)
	{
		buffer = frame_buffer(handles->shm, handles->width_max, handles->height_max, 0);
	}
	else if(handles->maxscreen)
	{
		buffer = frame_buffer(handles->shm, handles->width_max, handles->height_max, handles->border);
	}
	else {
		if(!handles->border) handles->border = DEFAULT_BORDER_WIDTH;
		buffer = frame_buffer(handles->shm, handles->width, handles->height, handles->border);
	}
	/* Draw window */
	wl_surface_attach(handles->z_index, buffer, 0, 0);
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

	uint8_t window_maximized = 0, window_fullscreen = 0;

	uint32_t *state;
	wl_array_for_each(state, states) {
		if(*state == XDG_TOPLEVEL_STATE_MAXIMIZED) window_maximized = 1;
		if(*state == XDG_TOPLEVEL_STATE_FULLSCREEN) window_fullscreen = 1;
	}

	DEBUG("sys Width: %d\n"
		"sys Height: %d\n"
		"client Width: %d\n"
		"client Height: %d\n",
		width, height, handles->width, handles->height
	);

	if(window_fullscreen || window_maximized)
	{
		DEBUG("Window state: Maximized or FullScreen\n");
		handles->width_max = width;
		handles->height_max = height;
	}
	else
	{
		DEBUG("Window state: Restored\n");
		handles->width = width > SD_SCREEN_WIDTH ? width : handles->width;
		handles->height = height > SD_SCREEN_HEIGHT ? height : handles->height;
	}
	handles->fullscreen = window_fullscreen;
	handles->maxscreen = window_maximized;
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
	int32_t width, int32_t height) { /* NOOP */ }/*
{
	struct interface_handles *handles = data;
	handles->width_max = width; handles->height_max = height;
}
*/
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

// Configure pointer and cursor
static void pointer_enter(void *data,
						 struct wl_pointer *pointer, uint32_t serial,
						 struct wl_surface *surface,
						 wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	struct interface_handles *handles = data;
	handles->pointer_serial = serial;

	wp_cursor_shape_device_v1_set_shape(
		handles->cursor_shape_device, handles->pointer_serial,
		WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT
	);

	handles->pointer_pos_x = wl_fixed_to_int(surface_x);
	handles->pointer_pos_y = wl_fixed_to_int(surface_y);
}
static void pointer_leave(void *data, struct wl_pointer *pointer,
	uint32_t serial, struct wl_surface *surface)
{
	struct interface_handles *handles = data;
	handles->pointer_serial = serial;
}
static void pointer_motion(void *data, struct wl_pointer *pointer,
	uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	struct interface_handles *handles = data;
	if(!handles->cursor_shape_device) return;

	wp_cursor_shape_device_v1_set_shape(
		handles->cursor_shape_device,
		handles->pointer_serial,
		WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT
	);

	handles->pointer_pos_x = wl_fixed_to_int(surface_x);
	handles->pointer_pos_y = wl_fixed_to_int(surface_y);
}
static void pointer_button(void *data, struct wl_pointer *pointer,
	uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	struct interface_handles *handles = data;
	handles->pointer_serial = serial;
	uint8_t window_resize_edge = 0;

	// close window button
	if(
		handles->pointer_pos_y <= DEFAULT_TITLEBAR_HEIGHT - handles->border &&
		handles->pointer_pos_x >= handles->width - (DEFAULT_TITLEBAR_HEIGHT << 1) &&
		handles->pointer_pos_x <= handles->width - handles->border
	)
	{
		handles->running = 0;
	}
	else if (
		handles->pointer_pos_x >= handles->border &&
		handles->pointer_pos_x <= handles->width - (DEFAULT_TITLEBAR_HEIGHT << 1) &&
		handles->pointer_pos_y <= DEFAULT_TITLEBAR_HEIGHT
	)
	{
		xdg_toplevel_move(handles->window, handles->seat, serial);
	}
	else
	{
		if (handles->pointer_pos_x >= 0 && handles->pointer_pos_x < handles->border)
			window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_LEFT;
		if (handles->pointer_pos_x > handles->width - handles->border && handles->pointer_pos_x <= handles->width)
			window_resize_edge = XDG_TOPLEVEL_RESIZE_EDGE_RIGHT;
		if (handles->pointer_pos_y >= handles->height - handles->border && handles->pointer_pos_y <= handles->height)
			window_resize_edge |= XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM;
	}
	if(window_resize_edge)
	{
		xdg_toplevel_resize(handles->window, handles->seat, serial, window_resize_edge);
	}
}/*
static void pointer_axis(void *data, struct wl_pointer *pointer,
	uint32_t time, uint32_t axis, wl_fixed_t value)
{}*/
static void pointer_frame(void *data, struct wl_pointer * pointer) {}
static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_enter,
	.leave = pointer_leave,
	.motion = pointer_motion,
	.button = pointer_button,/*
	.axis = pointer_axis,*/
	.frame = pointer_frame
};

// Configuring user input devices
static void
wl_seat_capabilities(void *data, struct wl_seat *seat,
	uint32_t capabilities)
{
	struct interface_handles *handles = data;

	if(capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
	{
		DEBUG("Keyboard detected\n");
		handles->keyboard = wl_seat_get_keyboard(seat);
	}

	if(capabilities & WL_SEAT_CAPABILITY_POINTER)
	{
		DEBUG("Mouse or TouchPad detected\n");
		handles->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(handles->pointer, &pointer_listener, handles);
		if(handles->cursor_shape_manager)
		{
			handles->cursor_shape_device = wp_cursor_shape_manager_v1_get_pointer(
				handles->cursor_shape_manager,
				handles->pointer);
		}
	}

	if(capabilities & WL_SEAT_CAPABILITY_TOUCH)
	{
		DEBUG("Touchscreen or graphics tablet detected\n");
	}
}

static void
wl_seat_name(void *data, struct wl_seat *seat,
	const char *name)
{
	DEBUG("User input device: %s\n", name);
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

	// Server side wayland cursor Not wayland-cursor
	if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0)
	{
		handles->cursor_shape_manager = wl_registry_bind(registry, name, &wp_cursor_shape_manager_v1_interface, version);
		DEBUG("Cursor Shape API is registered");
	}

	// Window Manager interface for window decoration and window resize message handling
	if (strcmp(interface, xdg_wm_base_interface.name) == 0)
	{
		handles->wm = wl_registry_bind(registry,
			name, &xdg_wm_base_interface, version);
		xdg_wm_base_add_listener(handles->wm, &xdg_listener, handles);
		handles->surface = xdg_wm_base_get_xdg_surface(handles->wm, handles->z_index);
		handles->window = xdg_surface_get_toplevel(handles->surface);
		xdg_toplevel_set_title(handles->window, handles->title);
		xdg_toplevel_set_app_id(handles->window, handles->app_id_8);

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
	//handles.width = 800; handles.height = 450;
	//handles.width = 797; handles.height = 559;
	strcpy(handles.app_id_8, "wayform");
	handles.title = "Wayland Form";
	// Connect display
	struct wl_display *display = wl_display_connect(WAYLAND_DISPLAY);
	if (!display)
	{
		DEBUG("Failed to connect to Wayland display\n");
		return -1;
	} else DEBUG( "Wayland compositor connected\n");

	// Query registry for available wayland features
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, &handles);
	wl_display_roundtrip(display);
	wl_surface_commit(handles.z_index);
	wl_display_roundtrip(display);

	// Keeping non-ui process alive
	handles.running = UINT8_MAX;
	while (wl_display_dispatch(display) != -1) {
	    if (!handles.running) break;
	}

	// Closing display connection
	if(handles.cursor_shape_device) wp_cursor_shape_device_v1_destroy(handles.cursor_shape_device);
	if(handles.cursor_shape_manager) wp_cursor_shape_manager_v1_destroy(handles.cursor_shape_manager);
	wl_surface_destroy(handles.z_index);
	wl_registry_destroy(registry);
	wl_display_disconnect(display);
	return 0;
}
