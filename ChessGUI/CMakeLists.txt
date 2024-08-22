cmake_minimum_required(VERSION 3.11)
project(ChessGUI)

set(CMAKE_CXX_STANDARD 17)

# Find OpenGL (GL) library
find_package(OpenGL REQUIRED)

add_executable(ChessGUI
    src/main.cpp
    src/include/OpenGLEngine/IndexBuffer.cpp
    src/include/OpenGLEngine/Renderer.cpp
    src/include/OpenGLEngine/VertexBuffer.cpp   
    src/include/OpenGLEngine/VertexArray.cpp
    src/include/OpenGLEngine/Shader.cpp
    src/include/OpenGLEngine/Texture.cpp
    src/ChessGUI.cpp
    src/include/stb_image/stb_image.cpp
    src/include/imgui/imgui_impl_glfw_gl3.cpp
    src/include/imgui/imgui_demo.cpp
    src/include/imgui/imgui_draw.cpp
    src/include/imgui/imgui.cpp
)


target_link_libraries(ChessGUI PRIVATE glfw GLEW OpenGL::GL)


enable_testing()

