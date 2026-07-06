# Define unchanged global variables
CC = gcc
CFLAGS = -O2 -Wall
LIBS = -lwayland-client

SOURCE_DIR = source

STABLE = stable
STANDING = standing
UNSTABLE = unstable
XDG_API_STABLE = /usr/share/wayland-protocols/$(STABLE)/xdg-shell/xdg-shell.xml
XDG_API = $(XDG_API_STABLE)

API_HEADER = xdg-protocol.h
API_SOURCE = xdg-protocol.c

# Export them to any sub-make processes
export CC CFLAGS LIBS XDG_API API_HEADER API_SOURCE

.PHONY: all clean

all:
	$(MAKE) -C $(SOURCE_DIR) all

clean:
	$(MAKE) -C $(SOURCE_DIR) clean
