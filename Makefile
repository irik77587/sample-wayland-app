# Define unchanged global variables
CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lwayland-client

SOURCE_DIR = source

WAYLAND_PROTOCOLS_DIR = /usr/share/wayland-protocols/
STABLE = stable
STAGING = staging
UNSTABLE = unstable
XDG_API_STABLE = ${WAYLAND_PROTOCOLS_DIR}$(STABLE)/xdg-shell/xdg-shell.xml
XDG_API = $(XDG_API_STABLE)
CURSOR_SHAPE_V1 = ${WAYLAND_PROTOCOLS_DIR}${STAGING}/cursor-shape/cursor-shape-v1.xml
CURSOR_SHAPE_API = ${CURSOR_SHAPE_V1}

CURSOR_SHAPE_API_HEADER = cursor-shape.h
CURSOR_SHAPE_API_SOURCE = cursor-shape.c

XDG_API_HEADER = xdg-protocol.h
XDG_API_SOURCE = xdg-protocol.c

# Export them to any sub-make processes
export CC CFLAGS LIBS XDG_API XDG_API_HEADER XDG_API_SOURCE CURSOR_SHAPE_API CURSOR_SHAPE_API_HEADER CURSOR_SHAPE_API_SOURCE

.PHONY: all clean

all:
	$(MAKE) -C $(SOURCE_DIR) all

clean:
	$(MAKE) -C $(SOURCE_DIR) clean
