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

add_executable(multithreaded_test
#  Game Engine
    src/multi_core_perft.cpp
    src/bb_magic_nums.cpp
    src/bit_board.cpp
)

add_executable(multithreaded_alpha_beta
#  Game Engine
    src/multi_core_alpha_beta.cpp
    src/bb_magic_nums.cpp
    src/bit_board.cpp
)

find_package(OpenMP REQUIRED)
target_link_libraries(multithreaded_test PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(multithreaded_alpha_beta PRIVATE OpenMP::OpenMP_CXX)


enable_testing()

