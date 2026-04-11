emcc -o docs/index.html src/main.cpp src/asteroid.cpp src/ship.cpp src/shot.cpp -Os -std=c++23 -Wall \
    ./ray_wasm/libraylib.a -I. -I./include -lm -s USE_GLFW=3 -s ASYNCIFY --preload-file resources \
    --shell-file ./ray_wasm/minshell.html -DPLATFORM_WEB
