add_library(cardgen
    STATIC
        src/cardgen.c
)

target_include_directories(cardgen
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
