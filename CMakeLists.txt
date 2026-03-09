cmake_minimum_required(VERSION 3.28)
project(chess_engine)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(chess_engine src/main.cpp
        include/board.h
        include/types.h
        src/board.cpp
        include/move.h
        include/move_list.h)
