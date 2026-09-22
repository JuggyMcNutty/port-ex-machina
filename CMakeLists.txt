cmake_minimum_required(VERSION 3.16)
project(deusex-launcher C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)          # _GNU_SOURCE-style extensions (flock, strcasestr)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wshadow -Wvla -Wno-unused-parameter)

if(NOT DEFINED DXL_PLATFORM)
    set(DXL_PLATFORM "host")
endif()
message(STATUS "dxl: platform=${DXL_PLATFORM}")

# ---------------------------------------------------------------------------
# core -- the launcher contract. No SDL, no platform assumptions, no I/O policy.
# Everything here is specified by docs/re/; see docs/DESIGN.md.
# ---------------------------------------------------------------------------
add_library(dxl_core STATIC
    src/core/common.c
    src/core/ini.c
    src/core/strings.c
    src/core/cmdline.c
    src/core/paths.c
    src/core/config.c
    src/core/policy.c
    src/core/sentinel.c
    src/core/install.c
    src/core/instance.c
    src/core/relaunch.c
    src/core/renderers.c
    src/core/log.c
)
target_include_directories(dxl_core PUBLIC src)

# ---------------------------------------------------------------------------
# session layer -- the launcher's sequence, shared by both front ends.
# ---------------------------------------------------------------------------
add_library(dxl_app STATIC src/app.c src/platform/posix.c)
target_include_directories(dxl_app PUBLIC src)
target_link_libraries(dxl_app PUBLIC dxl_core)

# Headless driver: the whole contract with no display. Always built -- it is
# how the on-device behaviour gets checked over SSH.
add_executable(dxl-cli src/cli_main.c)
target_link_libraries(dxl-cli PRIVATE dxl_app)
if(NOT DXL_PLATFORM STREQUAL "host")
    add_custom_command(TARGET dxl-cli POST_BUILD
        COMMAND "${CMAKE_SOURCE_DIR}/scripts/check-abi.sh" "$<TARGET_FILE:dxl-cli>"
        COMMENT "checking glibc ABI against device glibc 2.33")
endif()

# ---------------------------------------------------------------------------
# frontend -- SDL2. The device's own build on the cross target, the distro's
# on the host. Skipped while the UI sources do not exist yet.
# ---------------------------------------------------------------------------
if(EXISTS "${CMAKE_SOURCE_DIR}/src/main.c")
    if(DXL_PLATFORM STREQUAL "host")
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_ttf)
        set(DXL_SDL_INCLUDE ${SDL2_INCLUDE_DIRS})
        set(DXL_SDL_LIBS    ${SDL2_LIBRARIES})
        set(DXL_SDL_DIRS    ${SDL2_LIBRARY_DIRS})
    else()
        set(DXL_SDL_INCLUDE "${DXL_DEVICE_SYSROOT}/include")
        set(DXL_SDL_LIBS    SDL2 SDL2_ttf)
        set(DXL_SDL_DIRS    "${DXL_DEVICE_SYSROOT}/lib")
    endif()

    add_executable(deusex-launcher
        src/main.c
        src/ui/ui.c
        src/ui/theme.c
        src/ui/screens.c
    )
    target_include_directories(deusex-launcher PRIVATE src ${DXL_SDL_INCLUDE} ${DXL_SDL_INCLUDE}/SDL2)
    target_link_directories(deusex-launcher PRIVATE ${DXL_SDL_DIRS})
    target_link_libraries(deusex-launcher PRIVATE dxl_app ${DXL_SDL_LIBS} m)

    # Development tool: renders each screen headlessly for review.
    if(DXL_PLATFORM STREQUAL "host")
        add_executable(dxl-shots tools/shots.c src/ui/ui.c src/ui/theme.c src/ui/screens.c)
        target_include_directories(dxl-shots PRIVATE src ${DXL_SDL_INCLUDE} ${DXL_SDL_INCLUDE}/SDL2)
        target_link_directories(dxl-shots PRIVATE ${DXL_SDL_DIRS})
        target_link_libraries(dxl-shots PRIVATE dxl_app ${DXL_SDL_LIBS} m)
    endif()

    if(NOT DXL_PLATFORM STREQUAL "host")
        # The device resolves these from /usr/lib and /usr/trimui/lib at run time.
        target_link_options(deusex-launcher PRIVATE
            -Wl,--unresolved-symbols=ignore-in-shared-libs
            -Wl,-rpath-link,${DXL_DEVICE_SYSROOT}/lib)
        add_custom_command(TARGET deusex-launcher POST_BUILD
            COMMAND "${CMAKE_SOURCE_DIR}/scripts/check-abi.sh" "$<TARGET_FILE:deusex-launcher>"
            COMMENT "checking glibc ABI against device glibc 2.33")
    endif()
endif()

# ---------------------------------------------------------------------------
# tests -- host only; core has no graphics dependency so they need no display.
# ---------------------------------------------------------------------------
if(DXL_PLATFORM STREQUAL "host")
    enable_testing()
    add_subdirectory(tests)
endif()
