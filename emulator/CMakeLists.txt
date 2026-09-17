cmake_minimum_required(VERSION 3.16)
project(gb_rp2c02_emulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(gbcrt_core
    src/bridge.c
    src/rp2c02_ext.c
    src/pattern_source.c
    src/ppm.c
)
target_include_directories(gbcrt_core PUBLIC src)

add_executable(gbcrt_emu src/main.c)
target_link_libraries(gbcrt_emu PRIVATE gbcrt_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_bridge tests/test_bridge.c)
    target_link_libraries(test_bridge PRIVATE gbcrt_core)
    add_test(NAME bridge_geometry COMMAND test_bridge)
endif()
