cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)

add_executable(ChessEngine
    src/main.cpp
    src/board.cpp
    src/move.cpp
    src/utils.cpp
)

enable_testing()

