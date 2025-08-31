CC = clang-19
CFLAGS = -std=c11 -Wall -Wextra -pedantic
LIBRAYLIB := ./lib/libraylib.a
ALLOCLIB := ./lib/alloc.a

.PHONY = run build-linux build-windows clean

run: build-linux 
	./asteroids

build-linux: 
	$(CC) $(CFLAGS) asteroids.c $(LIBRAYLIB) $(ALLOCLIB) -o asteroids -lm

clean:
	@rm -rf asteroids