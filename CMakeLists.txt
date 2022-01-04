cmake_minimum_required(VERSION 3.14)

project(Chessbot2 LANGUAGES C)

find_package(Threads REQUIRED)



add_executable(chessbot
    src/attacks.c
    src/attacks.h
    src/bitboard.c
    src/bitboard.h
    src/evaluation.c
    src/evaluation.h
    src/main.c
    src/movegen.c
    src/movegen.h
    src/position.c
    src/position.h
    src/search.c
    src/stack.c
    src/stack.h
    src/tables.c
    src/tables.h
    src/tests.c
    src/tests.h
    src/types.h
    src/uci.c
    src/uci.h
)

target_compile_features(chessbot PRIVATE c_std_11)
set_target_properties(chessbot PROPERTIES C_EXTENSIONS off)
target_link_libraries(chessbot PRIVATE Threads::Threads)


