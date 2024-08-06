#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"  
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Texture.h"

// Tests
#include "tests/TestClearColor.h"
#include "tests/TestTexture2D.h"
#include "tests/TestBatchRendering2D.h"
#include "tests/TestBatchColor.h"
#include "tests/TestBatchTexture.h"
#include "tests/TestBatchDynamic.h"

#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_glfw_gl3.h"

int main(void)
{   
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set the major version of OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set the minor version of OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Set the profile of OpenGL to core (No backwards compatibility)

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
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

    test::Test* currentTest = nullptr;
    test::TestMenu* testMenu = new test::TestMenu(currentTest);
    currentTest = testMenu;

    testMenu->RegisterTest<test::TestClearColor>("Clear Color");
    testMenu->RegisterTest<test::TestTexture2D>("Texture2D");
    testMenu->RegisterTest<test::TestBatchRendering2D>("Batch Rendering 2D");
    testMenu->RegisterTest<test::TestBatchColor>("Batch Color");
    testMenu->RegisterTest<test::TestBatchTexture>("Batch Texture");
    testMenu->RegisterTest<test::TestBatchDynamic>("Batch Dynamic");

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 1.0f)); // Set the clear color

        renderer.Clear(); // Clear the color buffer
        ImGui_ImplGlfwGL3_NewFrame();

        if (currentTest){
            currentTest->OnUpdate(0.0f);
            currentTest->OnRender();

            ImGui::Begin("Test");
            if (currentTest != testMenu && ImGui::Button("<-"))
            {
                delete currentTest;
                currentTest = testMenu;
            }
            currentTest->OnImGuiRender();
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}


