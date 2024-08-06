#include "ChessGUI.h"

#include "include/OpenGLEngine/Renderer.h"
#include "include/imgui/imgui.h"
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <random>
#include <array>

static float get_texture(int piece);
static std::array<std::array<int, 8>, 8> vflip_board(std::array<std::array<int, 8>, 8> board);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

ChessGUI::selected_square sSquare = {0, 0};

ChessGUI::ChessGUI(GLFWwindow* window):
    m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)))
{   
    m_Window = window;
    int nVertices = 1000;
    
    GLint texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    std::cout << "Max texture units: " << texture_units << std::endl;

    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    m_Proj = glm::ortho(0.0f, (float)m_windowWidth, 0.0f, (float)m_windowHeight, -1.0f, 1.0f);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // BOARD SQUARES
    unsigned int indices_board[64 * 6] = {0}; // 6 indices x 64 squares  
    // std::cout << "Indices location: " << indices_board << std::endl;
    get_board_indices(indices_board, 64);  
    // std::cout << "Indices location: " << indices_board << std::endl;
    // std::cout << "Indices location last: " << &indices_board[383] << std::endl;
    size_t num_elements_board = sizeof(indices_board) / sizeof(indices_board[0]);
    // std::cout << "Num elements: " << num_elements_board << std::endl;
    // std::cout << "----------------------------------------" << std::endl;

    Vertex board_vertices[64 * 4]; // 4 vertices per square
    // std::cout << "Board Vertices location: " << board_vertices << std::endl;
    get_board_vertices(board_vertices);
    // std::cout << "Board Vertices location: " << board_vertices << std::endl;    
    // std::cout << "Num elements: " << sizeof(board_vertices) / sizeof(board_vertices[0]) << std::endl;
    // std::cout << "----------------------------------------" << std::endl;


    // PIECES
    unsigned int indices_pieces[32 * 6] = {0}; // 4 vertices per square
    // std::cout << "Pieces indices location: " << indices_pieces << std::endl;
    get_board_indices(indices_pieces, 32);
    // std::cout << "Pieces indices location: " << indices_pieces << std::endl;
    size_t num_elements_pieces = sizeof(indices_pieces) / sizeof(indices_pieces[0]);
    // std::cout << "Num elements: " << num_elements_pieces << std::endl;
    // std::cout << "----------------------------------------" << std::endl;

    // TARGETS
    unsigned int indices_targets[32 * 6] = {0}; // 4 vertices per square
    // std::cout << "Targets indices location: " << indices_targets << std::endl;
    get_board_indices(indices_targets, 32);
    // std::cout << "Targets indices location: " << indices_targets << std::endl;
    size_t num_elements_targets = sizeof(indices_targets) / sizeof(indices_targets[0]);
    // std::cout << "Num elements: " << num_elements_targets << std::endl;

    Vertex target_vertices[32 * 4] = {0};
    // std::cout << "Targets Vertices location: " << target_vertices << std::endl;
    get_target_vertices(target_vertices);
    // std::cout << "Targets Vertices location: " << target_vertices << std::endl;
    // std::cout << "Num elements: " << sizeof(target_vertices) / sizeof(target_vertices[0]) << std::endl;

    // Vertex target_vertices[1 * 4] = {0};
    // auto q = CreateQuad(0.0, 0.0, square_size, 0.0);

    // memcpy(target_vertices, q.data(),  q.size() * sizeof(Vertex));
    
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
    m_IndexBuffer_Pieces = std::make_unique<IndexBuffer>(indices_pieces, num_elements_pieces); // Instantiate an index buffer

    // Highlighting Squares will be Dynamic so we can move them constantly
    m_VAO_Targets = std::make_unique<VertexArray>(); // Instantiate a vertex array

    m_VertexBuffer_Targets = std::make_unique<VertexBuffer>(nullptr, sizeof(float) * 10 * 1 * 4, GL_DYNAMIC_DRAW); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Targets->addBuffer(*m_VertexBuffer_Targets, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Targets = std::make_unique<IndexBuffer>(indices_targets, num_elements_targets); // Instantiate an index buffer

    
    // // Board will be Static so we can't move it
    m_VAO_Board = std::make_unique<VertexArray>(); // Instantiate a vertex array
    m_VertexBuffer_Board = std::make_unique<VertexBuffer>(board_vertices, sizeof(float) * 10 * 64 * 4); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Board->addBuffer(*m_VertexBuffer_Board, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Board = std::make_unique<IndexBuffer>(indices_board, num_elements_board); // Instantiate an index buffer

    std::cout << "VAO_Pieces location: " << &m_VAO_Pieces << std::endl;
    std::cout << "VAO_Board location: " << &m_VAO_Board << std::endl;
    std::cout << "VAO_Targets location: " << &m_VAO_Targets << std::endl;

    m_Shader = std::make_unique<Shader>("res/shaders/batch.shader");
    m_Shader->Bind(); // Select the program shader
    
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
    m_Textures.push_back(std::make_unique<Texture>("res/textures/square_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>("res/textures/square_black.png")); // Instantiate a texture

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

    GLCall(glClearColor(0.7f, 0.7f, 0.7f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    Vertex vertices[220]; // 32 quads are 128 vertices
    get_piece_vertices(vertices);
    
    m_VertexBuffer_Pieces->Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // std::random_device dev;
    // std::mt19937 rng(dev());
    // std::uniform_int_distribution<std::mt19937::result_type> dist2(1,2); // distribution in range [1, 6]

    // std::cout << dist2(rng) << std::endl;


    // for (int i = 0; i < m_Textures.size(); i++){
    //     glBindTextureUnit(i, m_Textures[i]->GetRendererID());  
    // }

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
    
    float ratio = (m_windowWidth / 2048.0f > m_windowHeight / 1080.0f) ? m_windowHeight / 1080.0f : m_windowWidth / 2048.0f;

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(
        ((m_windowWidth / 2) - (ratio * square_size * 4)), 
        ((m_windowHeight / 2) - (ratio * square_size * 4)), 0));

    model =  glm::scale(model, glm::vec3(ratio, ratio, 1.0f));   
    m_Proj = glm::ortho(0.0f, (float)m_windowWidth, 0.0f, (float)m_windowHeight, -1.0f, 1.0f);
    glm::mat4 mvp =  m_Proj * m_View * model;

    m_Shader->Bind();
    m_Shader->SetUniformMat4f("u_MVP", mvp);

    renderer.Draw(*m_VAO_Board, *m_IndexBuffer_Board, *m_Shader);
    renderer.Draw(*m_VAO_Pieces, *m_IndexBuffer_Pieces, *m_Shader);
    renderer.Draw(*m_VAO_Targets, *m_IndexBuffer_Targets, *m_Shader);


}

void ChessGUI::OnImGuiRender()
{
    ImGui::DragFloat2("Quad Position", m_QuadPosition, 1.0f, 0.0f, 960.0f);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // std::cout << "Mouse button: " << button << " Action: " << action << " X: " << xpos << " Y: " << ypos << std::endl;
    sSquare.x = xpos;
    sSquare.y = ypos;
}

void ChessGUI::set_board(std::array<std::array<int, 8>, 8> board){
    this->board = board;
}

void ChessGUI::get_board_indices(unsigned int* indices_board, int n_squares)
{
    for (int i = 0; i < n_squares; i++){
        
        int stIdx = i * 4;
        int stdIdxArray = i * 6;

        indices_board[stdIdxArray] = stIdx;
        indices_board[stdIdxArray + 1] = stIdx + 1;
        indices_board[stdIdxArray + 2] = stIdx + 2;
        indices_board[stdIdxArray + 3] = stIdx + 2;
        indices_board[stdIdxArray + 4] = stIdx + 3;
        indices_board[stdIdxArray + 5] = stIdx;
    }
}
void ChessGUI::get_board_vertices(Vertex* vertices)
{
    int offset = 0;
    for (int i=0; i<8; i++)
    {  
        for (int j=0; j<8; j++){

            float texture = (i + j) % 2 == 0 ? 13.0f : 14.0f;
            
            auto q = CreateQuad((square_size * j), (square_size * i), square_size, texture);

            memcpy(vertices + offset, q.data(),  q.size() * sizeof(Vertex));
            offset += q.size();
        } 
    }
}

void ChessGUI::get_target_vertices(Vertex* vertices)
{   
    int offset = 0;
    for (int i=0; i<8; i++)
    {  
        for (int j=0; j<8; j++){
            auto q = CreateQuad((square_size), (square_size), square_size, 0.0);

            memcpy(vertices + offset, q.data(),  q.size() * sizeof(Vertex));
            offset += q.size();
        } 
    }

}

void ChessGUI::get_piece_vertices(Vertex* pieces_vertices){

    auto flipped_board = vflip_board(board);

    int quad_count = 0;
    int offset = 0;
    for (int i=0; i<8; i++)
    {  
        for (int j=0; j<8; j++){

            if (flipped_board[i][j] != 0){          
                float texture = flipped_board[i][j] > 0 ? (float)flipped_board[i][j]: (float)abs(board[i][j]) + 6.0;
                // std::cout << "Texture: " << texture << std::endl;
                quad_count++;
                auto q = CreateQuad((square_size * j), (square_size * i), square_size, texture);

                memcpy(pieces_vertices + offset, q.data(),  q.size() * sizeof(Vertex));
                offset += q.size();
            }
        } 
    }   
}   

static std::array<std::array<int, 8>, 8> vflip_board(std::array<std::array<int, 8>, 8> board){
    std::array<std::array<int, 8>, 8> new_board;
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){
            new_board[i][j] = board[7-i][j];
        }
    }
    return new_board;
}