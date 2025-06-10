add_library(mcfat
    STATIC
        src/mcio.c
        src/util.c
        src/mcfat.c
)

target_include_directories(mcfat
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
