# Common variables
SRC_DIR = src
BUILD_DIR = build
WEB_BUILD_DIR = build_web
RAYLIB_WEB_DIR = $(BUILD_DIR)/raylib_web
RAYGUI_WEB_DIR = $(BUILD_DIR)/raygui_web
TARGET = carousel

# Native build configuration
CC = gcc
CFLAGS = -Wall -Wextra -O2 
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Web build configuration
EMCC = emcc
EMFLAGS = -Wall -Wextra -O2 -I$(RAYLIB_WEB_DIR)/src -I$(RAYGUI_WEB_DIR)/src -DPLATFORM_WEB
EMLDFLAGS = -s USE_GLFW=3 -s WASM=1 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 \
            -s INITIAL_MEMORY=67108864 \
            -s EXPORTED_RUNTIME_METHODS=ccall \
            -s MINIFY_HTML=0 \
            --shell-file shell.html

.PHONY: all clean web raylib_web raygui_web rebuild

# Default target (native build)
all: $(BUILD_DIR)/$(TARGET)

# Native build rules
$(BUILD_DIR)/$(TARGET): $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS)

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

# Web build target
web: raylib_web raygui_web
	@mkdir -p $(WEB_BUILD_DIR)
	$(EMCC) $(SRC_DIR)/main.c -o $(WEB_BUILD_DIR)/index.html $(EMFLAGS) $(EMLDFLAGS) $(RAYLIB_WEB_DIR)/src/libraylib.a

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR) $(WEB_BUILD_DIR)

# Rebuild everything
rebuild: clean all