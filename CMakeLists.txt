cmake_minimum_required(VERSION 3.13)

project(ps2mcfs-wasm)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(mcfat)
add_subdirectory(cardgen)

add_executable(mcfs main.cpp)

target_link_libraries(mcfs
    PUBLIC
        embind
        mcfat
        cardgen
)

set_target_properties(mcfs PROPERTIES LINK_FLAGS "-O3 -s WASM=1 -s MODULARIZE=1 -s EXPORT_ES6=1 -s EXPORT_NAME=createModule --no-entry --extern-pre-js ${CMAKE_CURRENT_SOURCE_DIR}/extern-pre.js -s INITIAL_HEAP=8912896 -s ALLOW_MEMORY_GROWTH=1 -s MEMORY_GROWTH_GEOMETRIC_STEP=1.0")

configure_file(interface.d.ts mcfs.d.ts COPYONLY)
