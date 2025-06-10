cmake_minimum_required(VERSION 3.13)

project(ps2mcfs-wasm)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(mcfat)

add_executable(mcfs main.cpp)

target_link_libraries(mcfs
    PUBLIC
        embind
        mcfat
)

set_target_properties(mcfs PROPERTIES LINK_FLAGS "-O3 -s WASM=1 -s MODULARIZE=1 --no-entry")
