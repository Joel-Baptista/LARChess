cmake_minimum_required(VERSION 3.11)
project(BBChessGame)

# Find the GLFW library
# set(glfw_DIR "/home/joel/projects/glfw")
# find_package(glfw3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

# Find OpenGL (GL) library
find_package(OpenGL REQUIRED)

set(PROJECT_ROOT_DIR /home/joel/projects/YACE)

add_executable(BBChessGame
    ${PROJECT_ROOT_DIR}/BBChessGame/src/main.cpp
#  Game Engine
    ${PROJECT_ROOT_DIR}/BBChessEngine/src/bb_magic_nums.cpp
    ${PROJECT_ROOT_DIR}/BBChessEngine/src/bit_board.cpp
    ${PROJECT_ROOT_DIR}/utils/utils.cpp

# # Game GUI in OpenGL
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/IndexBuffer.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/Renderer.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/VertexBuffer.cpp   
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/VertexArray.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/Shader.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/OpenGLEngine/Texture.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/ChessGUI.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/stb_image/stb_image.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/imgui/imgui_impl_glfw_gl3.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/imgui/imgui_demo.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/imgui/imgui_draw.cpp
    ${PROJECT_ROOT_DIR}/ChessGUI/src/include/imgui/imgui.cpp
)




target_link_libraries(BBChessGame PRIVATE glfw GLEW OpenGL::GL)

enable_testing()

