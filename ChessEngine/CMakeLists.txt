cmake_minimum_required(VERSION 3.11)
project(ChessEngine)

# Find the GLFW library
# set(glfw_DIR "/home/joel/projects/glfw")
# find_package(glfw3 REQUIRED)

set(PROJECT_ROOT_DIR /home/joel/projects/YACE)

set(CMAKE_CXX_STANDARD 17)

# Find OpenGL (GL) library
find_package(OpenGL REQUIRED)

add_executable(ChessEngine
    ${PROJECT_ROOT_DIR}/ChessEngine/src/main.cpp
    ${PROJECT_ROOT_DIR}/ChessEngine/src/board.cpp
    ${PROJECT_ROOT_DIR}/ChessEngine/src/move.cpp
    ${PROJECT_ROOT_DIR}/ChessEngine/src/minmax.cpp
    ${PROJECT_ROOT_DIR}/utils/utils.cpp
)


enable_testing()

