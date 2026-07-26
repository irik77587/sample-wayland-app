BUILD_DIR=../build
LIB_SOURCE_DIR=lowlib
TARGET_LIB_DIR=../$(BUILD_DIR)
SOURCE=main.cpp
TARGET=$(BUILD_DIR)/lowlib-demo
TARGET_LIB_BIN=lib$(LIB_SOURCE_DIR).so

export TARGET_LIB_DIR TARGET_LIB_BIN

.PHONY: dev build run clean clean_all

dev: CFLAGS=-g -DDEVBUILD
dev: $(TARGET)

build: CFLAGS=-O3
build: $(TARGET)

$(BUILD_DIR)/$(TARGET_LIB_BIN):
	$(MAKE) -C $(LIB_SOURCE_DIR) $(TARGET_LIB_DIR)/$(TARGET_LIB_BIN)

.ONESHELL:
$(TARGET): $(SOURCE) $(BUILD_DIR)/$(TARGET_LIB_BIN)
	export LD_LIBRARY_PATH=$(shell realpath $(BUILD_DIR))
	gcc $(CFLAGS) $(SOURCE) -o $(TARGET) -L$(BUILD_DIR) -llowlib

.ONESHELL:
run: dev
	export LD_LIBRARY_PATH=$(BUILD_DIR)
	export WAYLAND_DEBUG=1
	$(TARGET)

clean:
	rm -f $(TARGET)

clean_all:
	$(MAKE) -C $(LIB_SOURCE_DIR) clean
	$(MAKE) clean
