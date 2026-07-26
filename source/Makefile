BUILD_DIR=../build
LIB_SOURCE_DIR=lowlib
TARGET_LIB_DIR=../$(BUILD_DIR)
SOURCE=main.cpp
TARGET=$(BUILD_DIR)/lowlib-demo
TARGET_LIB_BIN=lib$(LIB_SOURCE_DIR).so
CFLAGS?=-g -DDEVBUILD

export TARGET_LIB_DIR TARGET_LIB_BIN CFLAGS

.PHONY: dev build run clean lib

dev: $(TARGET)

build: CFLAGS=-O3
build: $(TARGET)

lib:
	$(MAKE) -C $(LIB_SOURCE_DIR) $(TARGET_LIB_DIR)/$(TARGET_LIB_BIN)

.ONESHELL:
$(TARGET): $(SOURCE) lib
	export LD_LIBRARY_PATH=$(shell realpath $(BUILD_DIR))
	gcc $(CFLAGS) $(SOURCE) -o $(TARGET) -L$(BUILD_DIR) -llowlib

.ONESHELL:
run: dev
	export LD_LIBRARY_PATH=$(BUILD_DIR)
	export WAYLAND_DEBUG=1
	$(TARGET)

clean:
	$(MAKE) -C $(LIB_SOURCE_DIR) clean
	rm -f $(TARGET)
