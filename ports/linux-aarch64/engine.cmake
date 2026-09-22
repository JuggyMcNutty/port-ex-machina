# Initial cache for Surreal Engine on aarch64 Linux, built natively on the
# device (scripts/engine.sh build linux-aarch64 there). Same as desktop Linux:
# run-game.sh picks the SDL2 backend, the one with gamepad support.
set(CMAKE_BUILD_TYPE Release CACHE STRING "")
set(ENABLE_SDL2    ON CACHE BOOL "")
set(ENABLE_SDL3    ON CACHE BOOL "")
set(ENABLE_X11     ON CACHE BOOL "")
set(ENABLE_WAYLAND ON CACHE BOOL "")
