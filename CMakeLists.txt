cmake_minimum_required(VERSION 3.11)
project(game)

# Find the GLFW library
# set(glfw_DIR "/home/joel/projects/glfw")
find_package(glfw3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# Find OpenGL (GL) library
find_package(OpenGL REQUIRED)

add_executable(ChessEngine
    src/main.cpp
    src/board.cpp
    src/move.cpp
    src/utils.cpp
    src/minmax.cpp
)

add_executable(game
    game/main.cpp
    src/board.cpp
    src/move.cpp
    src/utils.cpp
    src/minmax.cpp
)

# Link GLFW and OpenGL libraries to your executable
target_link_libraries(game PRIVATE glfw OpenGL::GL)

enable_testing()

