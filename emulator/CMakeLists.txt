cmake_minimum_required(VERSION 3.16)
project(gb_rp2c02_emulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(GBCRT_ENABLE_SAMEBOY "Build the optional SameBoy-backed ROM source" OFF)
option(GBCRT_ENABLE_SDL2 "Build the optional live SDL2 comparison viewer" OFF)
option(GBCRT_ENABLE_NESEMU_PPU "Use pinned johnmph/NESEmu Ricoh2C02 for the preview PPU" OFF)
set(NESEMU_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/NESEmu" CACHE PATH "Path to the pinned johnmph/NESEmu checkout")
set(SAMEBOY_ROOT "" CACHE PATH "Path to a SameBoy checkout with build/lib/libsameboy.a")

add_library(gbcrt_core
    src/adapter_palette.c
    src/bridge.c
    src/clock_mode.c
    src/source_model.c
    src/rp2c02_ext.c
    src/rp2c02_timing.c
    src/gbcrt_alt_video.c
    src/pattern_source.c
    src/gb_source.c
    src/gb_source_pattern.c
    src/comparison_render.c
    src/ui_font.c
    src/ppm.c
)
target_include_directories(gbcrt_core PUBLIC src)

if(GBCRT_ENABLE_NESEMU_PPU)
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    if(NOT EXISTS "${NESEMU_ROOT}/Source/NESEmu/Ppu.hpp")
        message(FATAL_ERROR
            "Pinned NESEmu PPU not found under ${NESEMU_ROOT}. Run emulator/scripts/bootstrap_nesemu.sh first or set -DNESEMU_ROOT=/path/to/NESEmu.")
    endif()

    target_sources(gbcrt_core PRIVATE
        src/rp2c02_nesemu.cpp
        "${NESEMU_ROOT}/Source/NESEmu/Common.cpp")
    target_include_directories(gbcrt_core PRIVATE "${NESEMU_ROOT}/Source")
    target_compile_definitions(gbcrt_core PUBLIC GBCRT_ENABLE_NESEMU_PPU=1)
endif()

if(GBCRT_ENABLE_SAMEBOY)
    if(NOT SAMEBOY_ROOT)
        message(FATAL_ERROR "GBCRT_ENABLE_SAMEBOY=ON requires -DSAMEBOY_ROOT=/path/to/SameBoy")
    endif()

    find_library(SAMEBOY_LIBRARY
        NAMES sameboy libsameboy
        PATHS "${SAMEBOY_ROOT}/build/lib"
        NO_DEFAULT_PATH)

    if(NOT SAMEBOY_LIBRARY)
        message(FATAL_ERROR
            "SameBoy library not found under ${SAMEBOY_ROOT}/build/lib. Build build/lib/libsameboy.a first.")
    endif()

    if(NOT EXISTS "${SAMEBOY_ROOT}/Core/gb.h")
        message(FATAL_ERROR
            "SameBoy Core/gb.h not found under ${SAMEBOY_ROOT}.")
    endif()

    add_library(SameBoy::Core UNKNOWN IMPORTED)
    set_target_properties(SameBoy::Core PROPERTIES
        IMPORTED_LOCATION "${SAMEBOY_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${SAMEBOY_ROOT}")

    target_sources(gbcrt_core PRIVATE
        src/gb_source_sameboy.c
        src/sameboy_bootroms.c)
    target_compile_definitions(gbcrt_core PRIVATE
        GB_INTERNAL=1
        GB_DISABLE_REWIND=1
        GB_DISABLE_DEBUGGER=1
        GB_DISABLE_CHEATS=1
        GB_DISABLE_CHEAT_SEARCH=1)
    target_compile_definitions(gbcrt_core PUBLIC GBCRT_ENABLE_SAMEBOY=1)
    target_link_libraries(gbcrt_core PUBLIC SameBoy::Core)
endif()

# Keep libm after optional static archives. GNU ld resolves static archives from
# left to right, and SameBoy itself references sin/cos/pow/round/etc.
if(UNIX)
    target_link_libraries(gbcrt_core PUBLIC m)
endif()

add_executable(gbcrt_emu src/main.c)
target_link_libraries(gbcrt_emu PRIVATE gbcrt_core)

if(GBCRT_ENABLE_SDL2)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

    add_executable(gbcrt_viewer src/viewer_sdl.c)
    target_link_libraries(gbcrt_viewer PRIVATE gbcrt_core PkgConfig::SDL2)
endif()

include(CTest)
if(BUILD_TESTING)
    # The regression suite intentionally uses assert(). Keep those checks live
    # even when a CI/configuration build selects Release and defines NDEBUG.
    if(MSVC)
        add_compile_options(/UNDEBUG)
    else()
        add_compile_options(-UNDEBUG)
    endif()

    add_executable(test_bridge tests/test_bridge.c)
    target_link_libraries(test_bridge PRIVATE gbcrt_core)
    add_test(NAME bridge_geometry COMMAND test_bridge)

    add_executable(test_alt_video tests/test_alt_video.c)
    target_link_libraries(test_alt_video PRIVATE gbcrt_core)
    add_test(NAME alternate_video_module COMMAND test_alt_video)

    add_executable(test_source tests/test_source.c)
    target_link_libraries(test_source PRIVATE gbcrt_core)
    add_test(NAME source_abstraction COMMAND test_source)

    add_executable(test_rp2c02_ext tests/test_rp2c02_ext.c)
    target_link_libraries(test_rp2c02_ext PRIVATE gbcrt_core)
    add_test(NAME rp2c02_ext_palette COMMAND test_rp2c02_ext)

    add_executable(test_adapter_palette tests/test_adapter_palette.c)
    target_link_libraries(test_adapter_palette PRIVATE gbcrt_core)
    add_test(NAME adapter_palette_presets COMMAND test_adapter_palette)

    add_executable(test_rp2c02_timing tests/test_rp2c02_timing.c)
    target_link_libraries(test_rp2c02_timing PRIVATE gbcrt_core)
    add_test(NAME rp2c02_timing COMMAND test_rp2c02_timing)

    add_executable(test_clock_mode tests/test_clock_mode.c)
    target_link_libraries(test_clock_mode PRIVATE gbcrt_core)
    add_test(NAME clock_mode COMMAND test_clock_mode)

    add_executable(test_palette_editor tests/test_palette_editor.c)
    target_link_libraries(test_palette_editor PRIVATE gbcrt_core)
    add_test(NAME palette_editor_hitboxes COMMAND test_palette_editor)

    if(GBCRT_ENABLE_NESEMU_PPU)
        add_executable(test_rp2c02_nesemu tests/test_rp2c02_nesemu.c)
        target_link_libraries(test_rp2c02_nesemu PRIVATE gbcrt_core)
        add_test(NAME rp2c02_nesemu_ext_path COMMAND test_rp2c02_nesemu)
    endif()
endif()
