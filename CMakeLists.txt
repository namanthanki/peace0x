cmake_minimum_required(VERSION 3.16)
project(peace0x C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler warnings and optimization flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
endif()

include_directories(include)

set(CORE_SOURCES
    src/bitboard.c
    src/board.c
    src/attack.c
    src/move.c
    src/movegen.c
    src/makemove.c
    src/transposition.c
    src/evaluate.c
    src/platform.c
    src/search.c
    src/uci.c
    src/perft.c
    src/init.c
)

# Core engine static library
add_library(peace0x_core STATIC ${CORE_SOURCES})

# Main UCI executable
add_executable(peace0x src/main.c)
target_link_libraries(peace0x PRIVATE peace0x_core)

# Automated regression test
add_executable(perft_test tests/perft_suite.c)
target_link_libraries(perft_test PRIVATE peace0x_core)

enable_testing()
add_test(NAME PerftSuite COMMAND perft_test)
