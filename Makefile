CC = gcc 
CCFLAGS = -O3 -I$(SOURCE_DIR) $(shell pkg-config --cflags libnl-3.0 libnl-genl-3.0) #include the netlink headers  
LDFLAGS = $(shell pkg-config --libs libnl-3.0 libnl-genl-3.0) 
DEBUGFLAGS = -g -O0
BUILD_DIR = build
SOURCE_DIR = 802.11

SOURCES := $(shell find $(SOURCE_DIR) -name "*.c")
OBJECTS = $(SOURCES:$(SOURCE_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPENDENCIES = $(SOURCES:.c=.d)
TARGET = $(BUILD_DIR)/main

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c Makefile 
	mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDENCIES)


debug: CCFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

run:
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: run clean debug