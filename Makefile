run:
	@gcc spaceinvaders.c -lraylib -o spaceinvaders
	@./spaceinvaders
	@echo "cleaning up the binary"
	@rm -rf spaceinvaders