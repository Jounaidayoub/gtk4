CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk4)
DEBUG_FLAGS = -g
LIBS = $(shell pkg-config --libs gtk4)

BUILD_DIR = build
TARGET = $(BUILD_DIR)/gtk-app
SRC = main.c widgets.c

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

debug: $(BUILD_DIR) $(SRC)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -o $(TARGET) $(SRC) $(LIBS)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	$(TARGET)

.PHONY: all clean run debug
