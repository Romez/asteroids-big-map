.PHONY: clean

asteroids: main.cpp
	g++ -g -fsanitize=address -std=c++23 -Wall -I./include main.cpp -o main ./lib/libraylib.a -lm

clear:
	rm ./asteroids
