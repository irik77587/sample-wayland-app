BUILD_DIR=../build
XDG_SPEC_FILE=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_SOURCE=./inc/xdg-shell.c
XDG_HEADER=./inc/xdg-shell.h
LIB_SOURCE=connect.cpp cursor.cpp event.cpp registry.cpp renderer.cpp shm.cpp window.cpp $(XDG_SOURCE)
TARGET_LIB=$(BUILD_DIR)/liblowlib.so
SOURCE=main.cpp
TARGET=$(BUILD_DIR)/lowlib-demo
LFLAGS=$(shell pkg-config --libs wayland-client wayland-cursor)

.PHONY: dev build clean run

dev: CFLAGS=-g -DDEVBUILD
dev: $(TARGET)

build: CFLAGS=-O3
build: $(TARGET)

.ONESHELL:
run: dev
	export LD_LIBRARY_PATH=$(BUILD_DIR)
	export WAYLAND_DEBUG=1
	$(TARGET)

clean:
	rm -f $(BUILD_DIR)/libcursor-theme.so
	rm -f $(TARGET_LIB)
	rm -f $(TARGET)

.ONESHELL:
$(TARGET): $(SOURCE) $(TARGET_LIB)
	export LD_LIBRARY_PATH=$(shell realpath $(BUILD_DIR))
	gcc $(CFLAGS) $(SOURCE) -o $(TARGET) -L$(BUILD_DIR) -llowlib

$(TARGET_LIB): $(LIB_SOURCE) $(BUILD_DIR)/libcursor-theme.so
	gcc -shared -fPIC $(CFLAGS) $(LIB_SOURCE) -o $(TARGET_LIB) $(LFLAGS) -L$(BUILD_DIR) -lcursor-theme

$(BUILD_DIR)/libcursor-theme.so: cursor-theme.cpp
	mkdir -p $(BUILD_DIR)
	gcc -shared -fPIC $(shell pkg-config --cflags dbus-1) cursor-theme.cpp -o $@ $(shell pkg-config --libs dbus-1)

$(XDG_SOURCE): $(XDG_SPEC_FILE) $(XDG_HEADER)
	wayland-scanner private-code $(XDG_SPEC_FILE) $<

$(XDG_HEADER): $(XDG_SPEC_FILE)
	wayland-scanner client-header $(XDG_SPEC_FILE) $<