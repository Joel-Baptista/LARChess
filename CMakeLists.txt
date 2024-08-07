cmake_minimum_required(VERSION 3.11)
project(game)

# Find the GLFW library
# set(glfw_DIR "/home/joel/projects/glfw")
# find_package(glfw3 REQUIRED)

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

add_executable(ChessGame
#  Game Engine
    game/main.cpp
    src/board.cpp
    src/move.cpp
    src/utils.cpp
    src/minmax.cpp
# Game GUI in OpenGL
    game/include/OpenGLEngine/IndexBuffer.cpp
    game/include/OpenGLEngine/Renderer.cpp
    game/include/OpenGLEngine/VertexBuffer.cpp   
    game/include/OpenGLEngine/VertexArray.cpp
    game/include/OpenGLEngine/Shader.cpp
    game/include/OpenGLEngine/Texture.cpp
    # src/include/OpenGLEngine/tests/Test.cpp
    game/ChessGUI.cpp
    game/include/stb_image/stb_image.cpp
    game/include/imgui/imgui_impl_glfw_gl3.cpp
    game/include/imgui/imgui_demo.cpp
    game/include/imgui/imgui_draw.cpp
    game/include/imgui/imgui.cpp
)

add_executable(bit_board_test
#  Game Engine
    tests/bit_board_test.cpp
    src/bitboard/bb_magic_nums.cpp
    src/bitboard/bit_board.cpp
)

# Link GLFW and OpenGL libraries to your executable
# target_link_libraries(game PRIVATE )

target_link_libraries(ChessGame PRIVATE glfw GLEW OpenGL::GL)

enable_testing()

