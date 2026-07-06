BUILD_DIR = ../BUILD

TARGET = $(BUILD_DIR)/hello
SOURCE = main.c


.PHONY : clean all

all : $(TARGET)


$(TARGET) : $(SOURCE) $(API_SOURCE) $(API_HEADER)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SOURCE) $(API_SOURCE) -o $(TARGET) $(LIBS)

$(API_HEADER) : $(XDG_API)
	wayland-scanner client-header $(XDG_API) $(API_HEADER)

$(API_SOURCE) : $(XDG_API)
	wayland-scanner private-code $(XDG_API) $(API_SOURCE)

clean :
	rm -rf $(BUILD_DIR) $(API_HEADER) $(API_SOURCE)
