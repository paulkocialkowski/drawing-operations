# Tools

CC = gcc

# Project

NAME = drawing-operations

# Directories

BUILD = build
OUTPUT = .

# Sources

SOURCES = display.c window.c buffer.c buffer-png.c \
	  drawing.c operations.c drawing-operations.c xdg-shell-protocol.c
OBJECTS = $(SOURCES:.c=.o)
DEPS = $(SOURCES:.c=.d)

WAYLAND_PROTOCOLS_DATA_DIR = $(shell pkg-config --variable=pkgdatadir wayland-protocols)

# Compiler

CFLAGS = $(shell pkg-config --cflags cairo)
LDFLAGS = -lwayland-client -lm -lcairo

# Produced files

BUILD_OBJECTS = $(addprefix $(BUILD)/,$(OBJECTS))
BUILD_DEPS = $(addprefix $(BUILD)/,$(DEPS))
BUILD_BINARY = $(BUILD)/$(NAME)
BUILD_DIRS = $(sort $(dir $(BUILD_BINARY) $(BUILD_OBJECTS)))

OUTPUT_BINARY = $(OUTPUT)/$(NAME)
OUTPUT_DIRS = $(sort $(dir $(OUTPUT_BINARY)))

all: $(OUTPUT_BINARY)

$(BUILD_DIRS):
	@mkdir -p $@

$(BUILD_OBJECTS): $(BUILD)/%.o: %.c | $(BUILD_DIRS)
	@echo " CC     $<"
	@$(CC) $(CFLAGS) -MMD -MF $(BUILD)/$*.d -c $< -o $@

$(BUILD_BINARY): $(BUILD_OBJECTS)
	@echo " LINK   $@"
	@$(CC) $(CFLAGS) -o $@ $(BUILD_OBJECTS) $(LDFLAGS)

$(OUTPUT_DIRS):
	@mkdir -p $@

$(OUTPUT_BINARY): $(BUILD_BINARY) | $(OUTPUT_DIRS)
	@echo " BINARY $@"
	@cp $< $@

.PHONY: clean
clean:
	@echo " CLEAN"
	@rm -rf $(BUILD)
	@rm -rf $(OUTPUT_BINARY)

xdg-shell-client-protocol.h:
	@wayland-scanner public-code $(WAYLAND_PROTOCOLS_DATA_DIR)/stable/xdg-shell/xdg-shell.xml $@

xdg-shell-protocol.c: xdg-shell-client-protocol.h
	@wayland-scanner public-code $(WAYLAND_PROTOCOLS_DATA_DIR)/stable/xdg-shell/xdg-shell.xml $@

-include $(BUILD_DEPS)
