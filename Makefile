RAYLIB_PATH ?= ~/raylib/src
LIBRAYLIB := $(RAYLIB_PATH)/libraylib.a
SHELL_FILE := $(RAYLIB_PATH)/shell.html

run:
	@gcc asteroids.c -lraylib -o asteroids -lm
	@./asteroids
	@echo "cleaning up the binary"
	@rm -rf asteroids

wasm:
	emcc -o asteroids.html asteroids.c \
		-Os -Wall \
		~/raylib/src/libraylib.a \
		-I. -I ~/raylib/src/ \
		-L. -L ~/raylib/src/ \
		-s USE_GLFW=3 \
		-s ASYNCIFY \
		--shell-file ~/raylib/src/shell.html \
		--preload-file assets \
		-s TOTAL_STACK=64MB \
		-s INITIAL_MEMORY=128MB \
		-s ASSERTIONS=1 \
		--profiling \
		-DPLATFORM_WEB