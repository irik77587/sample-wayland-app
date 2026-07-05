CC:=gcc
XDG_BAK_DIR:=xdg-bak
SOURCE_DIR:=source
BUILD_DIR:=build

XDG_API_STABLE:=stable
XDG_API_STAGING:=staging
XDG_API_UNSTABLE:=unstable

XDG_SOURCE:=/usr/share/wayland-protocols/$(XDG_API_STABLE)/xdg-shell/xdg-shell.xml
XDG_TARGETS:=$(SOURCE_DIR)/xdg-shell.c $(SOURCE_DIR)/xdg-client.h
TARGET:=main

.PHONY: all debug release clean

# 1. Default to debug if no target is specified
all: debug

# 2. Target-specific variables. They apply to the target AND its prerequisites.
debug: BUILD_CONFIG := debug
debug: CFLAG := -ggdb
debug: $(BUILD_DIR)/debug/$(TARGET)

release: BUILD_CONFIG := release
release: CFLAG := -O2
release: $(BUILD_DIR)/release/$(TARGET)

# 3. Pattern match for the final binary destination
$(BUILD_DIR)/%/$(TARGET): $(SOURCE_DIR)/$(TARGET).c $(XDG_TARGETS)
	mkdir -p $(dir $@)
	$(CC) $< $(SOURCE_DIR)/xdg-shell.c -o $@ -lwayland-client $(CFLAG)

# ---

.ONESHELL:
$(XDG_TARGETS): ${XDG_SOURCE}
	if [ -d /usr/share/wayland-protocols ]; then \
		wayland-scanner private-code ${XDG_SOURCE} $(SOURCE_DIR)/xdg-shell.c \
		wayland-scanner client-header ${XDG_SOURCE} $(SOURCE_DIR)/xdg-shell.h \
		echo "[SUCCESS] System-wide development dependency resolved" \
	else \
		echo "[WARN] System-wide installation of dependency unmet: wayland-protocols is not installed system-wide (optional)" \
		if [ -f $(XDG_BAK_DIR)/xdg-shell.c ] && [ -f $(XDG_BAK_DIR)/xdg-shell.h ]; then \
			cp $(XDG_BAK_DIR)/xdg-shell.c $(SOURCE_DIR)/ \
			cp $(XDG_BAK_DIR)/xdg-shell.h $(SOURCE_DIR)/ \
			echo "[INFO] Using local copy of dependency to mitigate dependency" \
		else \
			echo "[ERROR] Local installation of dependency unmet"; \
			echo "[HELP] Install wayland-protocol system-wise to resolve dependency"; \
		fi; \
	fi;

clean:
	rm -rf $(BUILD_DIR)
