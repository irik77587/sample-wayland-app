TARGET_LIB_DIR?=../../build
TARGET_LIB_BIN?=liblowlib.so
XDG_SPEC_FILE=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_SOURCE=./inc/xdg-shell.c
XDG_HEADER=./inc/xdg-shell.h
LIB_SOURCE=connect.cpp cursor.cpp event.cpp registry.cpp renderer.cpp shm.cpp window.cpp $(XDG_SOURCE)
TARGET_LIB=$(TARGET_LIB_DIR)/$(TARGET_LIB_BIN)
LFLAGS=$(shell pkg-config --libs wayland-client wayland-cursor)

$(TARGET_LIB): $(LIB_SOURCE) $(TARGET_LIB_DIR)/libcursor-theme.so
	gcc -shared -fPIC $(CFLAGS) $(LIB_SOURCE) -o $(TARGET_LIB) $(LFLAGS) -L$(TARGET_LIB_DIR) -lcursor-theme

$(TARGET_LIB_DIR)/libcursor-theme.so: cursor-theme.cpp
	mkdir -p $(TARGET_LIB_DIR)
	gcc -shared -fPIC $(CFLAGS) $(shell pkg-config --cflags dbus-1) cursor-theme.cpp -o $@ $(shell pkg-config --libs dbus-1)

$(XDG_SOURCE): $(XDG_SPEC_FILE) $(XDG_HEADER)
	wayland-scanner private-code $(XDG_SPEC_FILE) $@

$(XDG_HEADER): $(XDG_SPEC_FILE)
	wayland-scanner client-header $(XDG_SPEC_FILE) $@

.PHONY: clean

clean:
	rm -f $(TARGET_LIB_DIR)/libcursor-theme.so
	rm -f $(TARGET_LIB)
