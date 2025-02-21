# Common variables
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
WEB_BUILD_DIR = build_web
RAYLIB_WEB_DIR = $(BUILD_DIR)/raylib_web
RAYGUI_WEB_DIR = $(BUILD_DIR)/raygui_web
TARGET = carousel

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/carousel.c \
          $(SRC_DIR)/input_handler.c \
          $(SRC_DIR)/storage.c \
          $(SRC_DIR)/theme_manager.c

# Object files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
OBJECTS_WEB = $(SOURCES:$(SRC_DIR)/%.c=$(WEB_BUILD_DIR)/%.o)

# Assets and web files
ASSETS_DIR = assets
WEB_ASSETS = $(ASSETS_DIR)/favicon.ico \
             $(ASSETS_DIR)/apple-touch-icon.png \
             $(ASSETS_DIR)/icon-192.png \
             $(ASSETS_DIR)/icon-192-maskable.png \
             $(ASSETS_DIR)/icon-512.png \
             $(ASSETS_DIR)/icon-512-maskable.png

# Native build configuration
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
INCLUDES = -I./raygui/src -I$(INCLUDE_DIR)

# Web build configuration
EMCC = emcc
EMFLAGS = -Wall -Wextra -O2 -I$(RAYLIB_WEB_DIR)/src -I$(RAYGUI_WEB_DIR)/src -I$(INCLUDE_DIR) -DPLATFORM_WEB
EMLDFLAGS = -s USE_GLFW=3 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 \
            -s INITIAL_MEMORY=67108864 \
            -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap","UTF8ToString","stringToUTF8","lengthBytesUTF8"]' \
            -s EXPORTED_FUNCTIONS='["_main","_malloc","_free","_ProcessKeyPress"]' \
            -s ENVIRONMENT=web \
            -s "EXTRA_EXPORTED_RUNTIME_METHODS=['getValue','setValue']" \
            -s NO_EXIT_RUNTIME=1 \
            -s MINIFY_HTML=0 \
            --shell-file shell.html \
			--js-library storage_web.js



.PHONY: all clean web web-debug debug raylib_web raygui_web rebuild local copy-web-assets

# Default target (native build)
all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG
debug: all

# Raylib web build
$(RAYLIB_WEB_DIR)/src/libraylib.a:
	@mkdir -p $(RAYLIB_WEB_DIR)
	@if [ ! -d "$(RAYLIB_WEB_DIR)/src" ]; then \
	    git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_WEB_DIR); \
	fi
	cd $(RAYLIB_WEB_DIR)/src && \
	$(EMCC) -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2 -DHOST_EMSCRIPTEN && \
	$(EMCC) -c utils.c -Os -Wall -DPLATFORM_WEB -DHOST_EMSCRIPTEN && \
	$(EMCC) -c raudio.c -Os -Wall -DPLATFORM_WEB -DHOST_EMSCRIPTEN && \
	emar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o utils.o raudio.o

raylib_web: $(RAYLIB_WEB_DIR)/src/libraylib.a

# Raygui web setup
$(RAYGUI_WEB_DIR)/src/raygui.h:
	@mkdir -p $(RAYGUI_WEB_DIR)
	@if [ ! -d "$(RAYGUI_WEB_DIR)/src" ]; then \
	    git clone --depth 1 https://github.com/raysan5/raygui.git $(RAYGUI_WEB_DIR); \
	fi

raygui_web: $(RAYGUI_WEB_DIR)/src/raygui.h

# Copy web assets
copy-web-assets:
	@mkdir -p $(WEB_BUILD_DIR)/assets
	@cp -r $(WEB_ASSETS) $(WEB_BUILD_DIR)/assets/
	@cp manifest.json $(WEB_BUILD_DIR)/
	@cp service-worker.js $(WEB_BUILD_DIR)/

# Web build target
web: raylib_web raygui_web copy-web-assets
	@mkdir -p $(WEB_BUILD_DIR)
	$(EMCC) $(SOURCES) -o $(WEB_BUILD_DIR)/index.html $(EMFLAGS) $(EMLDFLAGS) $(RAYLIB_WEB_DIR)/src/libraylib.a

# Debug web build
web-debug: EMFLAGS += -g4 -O0 -DDEBUG
web-debug: web

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR)

# Rebuild everything
rebuild: clean all

# Local development target
local: all
	./$(BUILD_DIR)/$(TARGET)