run:
	@gcc asteroids.c -lraylib -o asteroids -lm
	@./asteroids
	@echo "cleaning up the binary"
	@rm -rf asteroids