SOURCE_DIR=source

.PHONY: build dev run clean

build:
	$(MAKE) -C $(SOURCE_DIR) build

dev:
	$(MAKE) -C $(SOURCE_DIR) dev

clean:
	$(MAKE) -C $(SOURCE_DIR) clean

run:
	$(MAKE) -C $(SOURCE_DIR) run
