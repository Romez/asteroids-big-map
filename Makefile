.PHONY: clean

asteroids: main.c
	gcc -std=c2x -Wall -I./include main.c -o main ./lib/libraylib.a -lm

clear:
	rm ./asteroids
