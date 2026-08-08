BUILD_DIR=../build
LIB_SOURCE_DIR=lowlib
TARGET_LIB_DIR=../$(BUILD_DIR)
SOURCE=main.cpp
TARGET=$(BUILD_DIR)/lowlib-demo
TARGET_LIB_BIN=lib$(LIB_SOURCE_DIR).so
CFLAGS?=-g -DDEVBUILD
# relative path to dynamic library
RLPATH=-Wl,-rpath,'$$ORIGIN'/

export TARGET_LIB_DIR TARGET_LIB_BIN CFLAGS RLPATH

.PHONY: dev build run clean clean_all lib

dev: $(TARGET)

build: CFLAGS=-O3
build: $(TARGET)

lib:
	$(MAKE) -C $(LIB_SOURCE_DIR) $(TARGET_LIB_DIR)/$(TARGET_LIB_BIN)

.ONESHELL:
$(TARGET): $(SOURCE) lib
	export LD_LIBRARY_PATH=$(shell realpath $(BUILD_DIR))
	gcc $(CFLAGS) $(RLPATH) $(SOURCE) -o $(TARGET) -L$(BUILD_DIR) -llowlib

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
