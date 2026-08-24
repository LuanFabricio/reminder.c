BUILD_FOLDER := build
TARGET := $(BUILD_FOLDER)/main
DEBUGGER = gf2
CC = gcc
CINCLUDES = -Iinclude -Llibs
CLINKS = -lm -lpthread -lcurl -ltelebot -ljson-c
CFLAGS := $(CINCLUDES) $(CLINKS) -ggdb

SRC_DIR := src/
SRC_FILES := $(shell find $(SRC_DIR) -type f -name '*.c')

all: $(BUILD_FOLDER)
	$(CC) -o $(TARGET) $(SRC_FILES) $(CFLAGS)

$(BUILD_FOLDER):
	mkdir -p $(BUILD_FOLDER)

run: all
	$(TARGET)

debug: all
	$(DEBUGGER) $(TARGET)
