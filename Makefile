SOURCE_DIR=source
TARGET=../build/metaform
# TARGET is relative to source directory, not this/project directory
export TARGET

.PHONY: build dev run clean

build:
	$(MAKE) -C $(SOURCE_DIR) build

dev:
	$(MAKE) -C $(SOURCE_DIR) dev

clean:
	$(MAKE) -C $(SOURCE_DIR) clean

run:
	$(MAKE) -C $(SOURCE_DIR) run
