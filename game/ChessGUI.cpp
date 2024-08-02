#include "ChessGUI.h"

#include "include/OpenGLEngine/Renderer.h"
#include "include/imgui/imgui.h"
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <random>
#include <array>

struct Vertex {
        float Position[3];
        float Color[4];
        float TexCoords[2];
        float TexID;
    };

static std::array<Vertex, 4> CreateQuad(
    float x, float y, float size, 
    float textureID, // x and y are the left bottom corner of the quad
    float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 0.0f) 
{
    Vertex v0;
    v0.Position[0] = x; v0.Position[1] = y; v0.Position[2] = 0.0f;
    v0.Color[0] = r; v0.Color[1] = g; v0.Color[2] = b; v0.Color[3] = a;
    v0.TexCoords[0] = 0.0f; v0.TexCoords[1] = 0.0f;
    v0.TexID = textureID; 

    Vertex v1;
    v1.Position[0] = x + size; v1.Position[1] = y; v1.Position[2] = 0.0f;
    v1.Color[0] = r; v1.Color[1] = g; v1.Color[2] = b; v1.Color[3] = a;
    v1.TexCoords[0] = 1.0f; v1.TexCoords[1] = 0.0f;
    v1.TexID = textureID; 

    Vertex v2;
    v2.Position[0] = x + size; v2.Position[1] = y + size; v2.Position[2] = 0.0f;
    v2.Color[0] = r; v2.Color[1] = g; v2.Color[2] = b; v2.Color[3] = a;
    v2.TexCoords[0] = 1.0f; v2.TexCoords[1] = 1.0f;
    v2.TexID = textureID; 

    Vertex v3;
    v3.Position[0] = x; v3.Position[1] = y + size; v3.Position[2] = 0.0f;
    v3.Color[0] = r; v3.Color[1] = g; v3.Color[2] = b; v3.Color[3] = a;
    v3.TexCoords[0] = 0.0f; v3.TexCoords[1] = 1.0f;
    v3.TexID = textureID; 

    return {v0, v1, v2, v3};
}

ChessGUI::ChessGUI(GLFWwindow* window):
    m_TranslationA(200, 200, 0), m_TranslationB(500, 200, 0), 
    m_Proj(glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f)), m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)))
{   
    m_Window = window;
    int nVertices = 1000;

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
        
        4, 5, 6,
        6, 7, 4,
    };

    size_t num_elements = sizeof(indices) / sizeof(indices[0]);

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function
    GLCall(glEnable(GL_BLEND)); // Enable blending

    VertexBufferLayout layout; // Instantiate a vertex buffer layout
    layout.Push<float>(3); // Push the positions to the layout
    layout.Push<float>(4); // Push the colors to the layout
    layout.Push<float>(2); // Push the texture positions to the layout
    layout.Push<float>(1); // Push the texture ids to the layout

    // Pieces will be Dynamic so we can move them constantly
    m_VAO_Pieces = std::make_unique<VertexArray>(); // Instantiate a vertex array

    m_VertexBuffer_Pieces = std::make_unique<VertexBuffer>(nullptr, sizeof(Vertex) * nVertices, GL_DYNAMIC_DRAW); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Pieces->addBuffer(*m_VertexBuffer_Pieces, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Pieces = std::make_unique<IndexBuffer>(indices, num_elements); // Instantiate an index buffer

    // Board will be Static so we can't move it
    m_VAO_Board = std::make_unique<VertexArray>(); // Instantiate a vertex array
    m_VertexBuffer_Board = std::make_unique<VertexBuffer>(nullptr, sizeof(Vertex) * nVertices, GL_DYNAMIC_DRAW); // Instantiate a buffer with the positions and binds it by default
    // VertexBufferLayout layout; // Instantiate a vertex buffer layout
    // layout.Push<float>(3); // Push the positions to the layout
    // layout.Push<float>(4); // Push the colors to the layout
    // layout.Push<float>(2); // Push the texture positions to the layout
    // layout.Push<float>(1); // Push the texture ids to the layout

    m_VAO_Board->addBuffer(*m_VertexBuffer_Board, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Board = std::make_unique<IndexBuffer>(indices, num_elements); // Instantiate an index buffer


    m_Shader = std::make_unique<Shader>("res/shaders/batch.shader");
    m_Shader->Bind(); // Select the program shader
    
    m_Textures.push_back(std::make_unique<Texture>("res/textures/square_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/square_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/pawn_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/knight_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/bishop_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/rook_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/queen_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/king_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/pawn_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/knight_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/bishop_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/rook_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/queen_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/king_black.png")); // Instantiate a texture

    int sampler[m_Textures.size()];
    
    for (int i = 0; i < m_Textures.size(); i++){
        glBindTextureUnit(i, m_Textures[i]->GetRendererID());  
        sampler[i] = i;
    }

    size_t size = sizeof(sampler) / sizeof(sampler[0]);
    m_Shader->SetUniform1iv("u_Textures", size, sampler); // That's how we select textures in the shader

}
ChessGUI::~ChessGUI()
{
    
}

void ChessGUI::OnUpdate(float deltaTime)
{

    auto q0 = CreateQuad(m_QuadPosition[0], m_QuadPosition[1], 50.0f, 3.0f);
    auto q1 = CreateQuad(150.0f, -50.0f, 50.0f, 4.0f);

    Vertex vertices[8]; // 2 quads are 8 vertices
    memcpy(vertices, q0.data(),  q0.size() * sizeof(Vertex));
    memcpy(vertices + q0.size(),  q1.data(), q1.size() * sizeof(Vertex));

    GLCall(glClearColor(0.7f, 0.7f, 0.7f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    m_VertexBuffer_Pieces->Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    // std::random_device dev;
    // std::mt19937 rng(dev());
    // std::uniform_int_distribution<std::mt19937::result_type> dist2(1,2); // distribution in range [1, 6]

    // std::cout << dist2(rng) << std::endl;

    for (int i = 0; i < m_Textures.size(); i++){
        glBindTextureUnit(i, m_Textures[i]->GetRendererID());  
    }

    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    
    if (width != m_windowWidth || height != m_windowHeight){
        m_windowWidth = width;
        m_windowHeight = height;
        glViewport(0, 0, width, height);
    }

}

void ChessGUI::OnRender()
{
    Renderer renderer;
    
    {   
        glm::mat4 model = glm::translate(glm::mat4(1.0f), m_TranslationA);
        glm::mat4 mvp = m_Proj * m_View * model;
        m_Shader->Bind();
        m_Shader->SetUniformMat4f("u_MVP", mvp);

        renderer.Draw(*m_VAO_Board, *m_IndexBuffer_Board, *m_Shader);
        renderer.Draw(*m_VAO_Pieces, *m_IndexBuffer_Pieces, *m_Shader);
    }

}

void ChessGUI::OnImGuiRender()
{
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

