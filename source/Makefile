SOURCE=canvas.cpp events.cpp listeners.cpp main.cpp register.cpp window.cpp xdg-protocol.c
LFLAGS=-lwayland-client -lwayland-cursor
CFLAGS=-g -DDEBUG

.PHONY:build run clean dev

dev: $(TARGET)

build: CFLAGS=-O3
build: $(TARGET)

run:
	$(TARGET)

clean:
	rm -f $(TARGET)


$(TARGET) : $(SOURCE)
	gcc $(CFLAGS) $(SOURCE) -o $(TARGET) $(LFLAGS)
