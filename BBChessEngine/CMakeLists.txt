cmake_minimum_required(VERSION 3.11)
project(BBChesEngine)

# Find the GLFW library
# set(glfw_DIR "/home/joel/projects/glfw")
# find_package(glfw3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_executable(BBChesEngine
#  Game Engine
    src/bit_board_test.cpp
    src/bb_magic_nums.cpp
    src/bit_board.cpp
)


enable_testing()

