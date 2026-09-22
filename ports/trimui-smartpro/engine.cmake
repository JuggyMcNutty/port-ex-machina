# Initial cache for Surreal Engine on the Smart Pro (scripts/engine.sh build).
#
# SDL2 only: the vendor SDL2 is the one display path on the device, and there
# is no X11, no Wayland compositor and no desktop GL. The toolchain links
# libstdc++ statically and keeps every symbol at or below glibc 2.33.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/toolchain-cxx.cmake" CACHE FILEPATH "")
set(CMAKE_BUILD_TYPE Release CACHE STRING "")
set(ENABLE_SDL3    OFF CACHE BOOL "")
set(ENABLE_SDL2    ON  CACHE BOOL "")
set(ENABLE_X11     OFF CACHE BOOL "")
set(ENABLE_WAYLAND OFF CACHE BOOL "")
