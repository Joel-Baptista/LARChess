// Misc Imports
#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>

// My OpenGL Engine
#include "include/OpenGLEngine/Renderer.h"
#include "include/OpenGLEngine/VertexBuffer.h"
#include "include/OpenGLEngine/IndexBuffer.h"
#include "include/OpenGLEngine/VertexArray.h"  
#include "include/OpenGLEngine/Shader.h"
#include "include/OpenGLEngine/VertexBufferLayout.h"
#include "include/OpenGLEngine/Texture.h"
#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

// glm for matrix transformations
#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

// ChessGUI
#include "ChessGUI.h"

struct BotResult{
    std::string move;
    double evaluation;
    bool processed = false;
};

int main() {

    GLFWwindow* window;
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    
    std::cout << "Number of monitors: " << count << std::endl;
    int width_mm, height_mm;
    // glfwGetMonitorPhysicalSize(primary, &width_mm, &height_mm);

    // std::cout << "Width: " << width_mm << "mm, Height: " << height_mm << "mm" << std::endl;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Set the profile of OpenGL to core (No backwards compatibility)

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Lar Chess", NULL, NULL);
    glfwSetWindowAspectRatio(window, 19, 10);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); // Enable VSync

    // Initialize GLEW only after creating the window
    if (glewInit() != GLEW_OK) 
        std::cout << "Error!" << std::endl;

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function
    GLCall(glEnable(GL_BLEND)); // Enable blending

    Renderer renderer; // Instantiate a renderer

    ImGui::CreateContext();

    ImGui_ImplGlfwGL3_Init(window, true);

    ImGui::StyleColorsDark();

    ChessGUI chessGUI(window, "/home/joel/projects/YACE/ChessGUI/res");
    
    // /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {   
        /* Render here */
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Set the clear color

        // Get the current window size

        renderer.Clear(); // Clear the color buffer
        ImGui_ImplGlfwGL3_NewFrame();

        chessGUI.OnUpdate(0.0f);

        chessGUI.OnRender();

        ImGui::Begin("Test");

        chessGUI.OnImGuiRender();
        ImGui::End();

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    
    }

     std::cout << "7.End Program" << std::endl;
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

}

