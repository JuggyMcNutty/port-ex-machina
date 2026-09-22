# Initial cache for Surreal Engine on desktop Linux (scripts/engine.sh build).
#
# Every display backend stays on; run-game.sh picks SDL2 at run time, because
# that is the backend with the fork's gamepad support.
set(CMAKE_BUILD_TYPE Release CACHE STRING "")
set(ENABLE_SDL2    ON CACHE BOOL "")
set(ENABLE_SDL3    ON CACHE BOOL "")
set(ENABLE_X11     ON CACHE BOOL "")
set(ENABLE_WAYLAND ON CACHE BOOL "")
