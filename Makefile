.PHONY: clean

asteroids: main.cpp
	g++ -fsanitize=address -std=c++23 -Wall -I./include main.cpp -o main ./lib/libraylib.a -lm

asteroid_builder:
	g++ -Wall -fsanitize=address -std=c++23 -I./includes asteroid_builder.cpp -o main ./lib/libraylib.a -lm

clear:
	rm ./asteroids
