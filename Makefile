.PHONY: clean

AST_SRC = src/main.cpp src/asteroid.cpp src/ship.cpp src/shot.cpp

asteroids: src/main.cpp
	g++ -fsanitize=address -std=c++23 -Wall -I./include $(AST_SRC) -o main ./lib/libraylib.a

asteroid_builder:
	g++ -Wall -fsanitize=address -std=c++23 -I./includes src/asteroid_builder.cpp -o main ./lib/libraylib.a -lm

clear:
	rm ./asteroids
