cmake_minimum_required(VERSION 3.13)

project(ps2mcfs-wasm)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

add_subdirectory(mcfat)
add_subdirectory(cardgen)

add_executable(mcfs main.cpp)

target_link_libraries(mcfs
    PUBLIC
        embind
        mcfat
        cardgen
)

target_link_options(mcfs
    PUBLIC
        "-sWASM=1"
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sEXPORT_NAME=createModule"
        "--no-entry"
        "--extern-pre-js=${CMAKE_CURRENT_SOURCE_DIR}/extern-pre.js"
        "-sINITIAL_MEMORY=8912896"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sMEMORY_GROWTH_GEOMETRIC_STEP=1.0"
)

configure_file(interface.d.ts mcfs.d.ts COPYONLY)
