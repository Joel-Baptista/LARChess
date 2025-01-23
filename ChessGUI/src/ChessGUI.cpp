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


ChessGUI::mouse_input iMouse;

ChessGUI::ChessGUI(GLFWwindow* window, std::string res_path):
    m_View(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0))), m_RatioX(1.0f), m_RatioY(1.0f)
{   
    m_Window = window;
    
    GLint texture_units;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);

    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    m_Proj = glm::ortho(0.0f, (float)m_windowWidth, 0.0f, (float)m_windowHeight, -1.0f, 1.0f);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // BOARD SQUARES
    unsigned int indices_board[64 * 6] = {0}; // 6 indices x 64 squares  
    get_board_indices(indices_board, 64);  
    size_t num_elements_board = sizeof(indices_board) / sizeof(indices_board[0]);

    Vertex board_vertices[64 * 4]; // 4 vertices per square
    get_board_vertices(board_vertices);

    // PIECES
    unsigned int indices_pieces[32 * 6] = {0}; // 4 vertices per square
    get_board_indices(indices_pieces, 32);
    size_t num_elements_pieces = sizeof(indices_pieces) / sizeof(indices_pieces[0]);

    // TARGETS
    unsigned int indices_targets[1 * 6] = {0}; // 4 vertices per square
    get_board_indices(indices_targets, 1);
    size_t num_elements_targets = sizeof(indices_targets) / sizeof(indices_targets[0]);

    Vertex target_vertices[1 * 4] = {0};
    auto q = CreateQuad(0.0, 0.0, square_size, 0.0);

    memcpy(target_vertices, q.data(),  q.size() * sizeof(Vertex));
    
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Set the blending function
    GLCall(glEnable(GL_BLEND)); // Enable blending

    VertexBufferLayout layout; // Instantiate a vertex buffer layout
    layout.Push<float>(3); // Push the positions to the layout
    layout.Push<float>(4); // Push the colors to the layout
    layout.Push<float>(2); // Push the texture positions to the layout
    layout.Push<float>(1); // Push the texture ids to the layout
    
    // Pieces will be Dynamic so we can move them constantly
    m_VAO_Pieces = std::make_unique<VertexArray>(); // Instantiate a vertex array
    
    m_VertexBuffer_Pieces = std::make_unique<VertexBuffer>(nullptr, sizeof(Vertex) * m_NumPieceVertex, GL_DYNAMIC_DRAW); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Pieces->addBuffer(*m_VertexBuffer_Pieces, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Pieces = std::make_unique<IndexBuffer>(indices_pieces, num_elements_pieces); // Instantiate an index buffer

    // Highlighting Squares will be Dynamic so we can move them constantly
    m_VAO_Targets = std::make_unique<VertexArray>(); // Instantiate a vertex array

    m_VertexBuffer_Targets = std::make_unique<VertexBuffer>(target_vertices, sizeof(float) * 10 * 1 * 4, GL_DYNAMIC_DRAW); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Targets->addBuffer(*m_VertexBuffer_Targets, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Targets = std::make_unique<IndexBuffer>(indices_targets, num_elements_targets); // Instantiate an index buffer

    
    // // Board will be Static so we can't move it
    m_VAO_Board = std::make_unique<VertexArray>(); // Instantiate a vertex array
    m_VertexBuffer_Board = std::make_unique<VertexBuffer>(board_vertices, sizeof(float) * 10 * 64 * 4); // Instantiate a buffer with the positions and binds it by default

    m_VAO_Board->addBuffer(*m_VertexBuffer_Board, layout); // Add the buffer to the vertex array
    m_IndexBuffer_Board = std::make_unique<IndexBuffer>(indices_board, num_elements_board); // Instantiate an index buffer

    m_Shader = std::make_unique<Shader>( res_path + "/shaders/batch.shader");
    m_Shader->Bind(); // Select the program shader
    
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/pawn_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/knight_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/bishop_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/rook_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/queen_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/king_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/pawn_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/knight_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/bishop_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/rook_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/queen_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/king_black.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/square_white.png")); // Instantiate a texture
    m_Textures.push_back(std::make_unique<Texture>(res_path + "/textures/square_black.png")); // Instantiate a texture

    int sampler[m_Textures.size()];
    
    for (int i = 0; i < m_Textures.size(); i++){
        glBindTextureUnit(i, m_Textures[i]->GetRendererID());  
        sampler[i] = i;
    }

    size_t size = sizeof(sampler) / sizeof(sampler[0]);
    m_Shader->SetUniform1iv("u_Textures", size, sampler); // That's how we select textures in the shader
    std::cout << "ChessGUI Initialized" << std::endl;
}
ChessGUI::~ChessGUI()
{
    
}

void ChessGUI::OnUpdate(float deltaTime)
{

    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    
    if (width != m_windowWidth || height != m_windowHeight){

        m_windowWidth = width;
        m_windowHeight = height;

        // Board Offset
        m_RatioX = (float)m_windowWidth / 1920.0f;
        m_RatioY = (float)m_windowHeight / 1080.0f;

        bOffsetX = (m_windowWidth / 2)  - (4 * square_size * m_RatioX);
        bOffsetY = (m_windowHeight / 2) - (4 * square_size * m_RatioY);
    }

    GLCall(glClearColor(0.7f, 0.7f, 0.7f, 1.0f));
    GLCall(glClear(GL_COLOR_BUFFER_BIT));

    // Draw Pieces
    Vertex vertices[m_NumPieceVertex]; // 32 quads are 128 vertices
    get_piece_vertices(vertices);
    
    m_VertexBuffer_Pieces->Bind();
    glClearBufferData(GL_ARRAY_BUFFER, GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    auto time_now = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = time_now - iMouse.press_time;

    int x = floor(((iMouse.x - bOffsetX)/ m_RatioX) / square_size);
    int y = floor((((m_windowHeight - bOffsetY) / m_RatioY) - (iMouse.y / m_RatioY)) / square_size);
    // Draw Selected Square
    if (y >= 0 && y<8 && x >= 0 && x<8 && !board_locked && iMouse.button == 1){
        if (iMouse.mode == GLFW_PRESS && !iMouse.updated){
            iMouse.updated = true;
            if (selected_square.piece != 0 && (x != selected_square.x || y != selected_square.y)){ // Movement
                m_PlayerMove = coords_to_square(7 - selected_square.y, selected_square.x) + coords_to_square(7 - y, x);
                
                board[7-y][x] = selected_square.piece;
                board[7-selected_square.y][selected_square.x] = 0;

                std::cout << "Piece: " << selected_square.piece << "X: " << x << "Y: " << y << std::endl; 
                if ((selected_square.piece == 1 && y == 7) || (selected_square.piece == -1 && y == 0)){
                    m_PlayerMove += "q";
                    board[7-y][x] = 5 * selected_square.piece;
                }
                move_stored = true;
                selected_square.selected = false;
                selected_square.piece = 0;
            }else if (x == selected_square.x && y == selected_square.y && selected_square.selected){ // Unselect
                board[7-y][x] = selected_square.piece;
                selected_square.piece = 0;
                selected_square.selected = false;
                m_PlayerMove = "";
            }else if (board[7-y][x] != 0){
                selected_square.piece = board[7-y][x];
                board[7-y][x] = 0;
                selected_square.selected = true;
                selected_square.only_select = false;
                m_PlayerMove = "";
            

            selected_square.x = x;
            selected_square.y = y;  
            }else{
                selected_square.piece = 0;
                selected_square.selected = false;
                m_PlayerMove = "";
            }      
        } else if (iMouse.mode == GLFW_RELEASE && !iMouse.updated && elapsed_seconds.count() < 0.025){
            iMouse.updated = true; 
            if (x == selected_square.x && y == selected_square.y && selected_square.selected){ // Select
                board[7-y][x] = selected_square.piece;
                selected_square.only_select = true;

                board[7-y][x] = selected_square.piece;
                board[7-selected_square.y][selected_square.x] = 0;

            }
        } else if (iMouse.mode == GLFW_RELEASE && !iMouse.updated){
            iMouse.updated = true;
            if (selected_square.piece != 0 && (x != selected_square.x || y != selected_square.y)){ // Movement
                m_PlayerMove = coords_to_square(7 - selected_square.y, selected_square.x) + coords_to_square(7 - y, x);
                
                board[7-y][x] = selected_square.piece;
                board[7-selected_square.y][selected_square.x] = 0;
                
                if ((selected_square.piece == 1 && y == 7) || (selected_square.piece == -1 && y == 0)){
                    m_PlayerMove += "q";
                    board[7-y][x] = 5 * selected_square.piece;
                }
                move_stored = true;
                selected_square.selected = false;
                selected_square.piece = 0;
            }     
        }
    }
    float color[4] = {1.0f, 0.5f, 0.5f, 0.5f};

    if (selected_square.piece == 0 || selected_square.only_select) color[3] = 0.0f;

    auto flipped_board = vflip_board(board);
    float texture = selected_square.piece > 0 ? 
                    (float)selected_square.piece: 
                    (float)abs(selected_square.piece) + 6.0;

    if (selected_square.piece == 0 || selected_square.only_select) texture = 0.0f;
    
    double xpos, ypos;
    glfwGetCursorPos(m_Window, &xpos, &ypos);

    auto q = CreateQuad(
        ((xpos - bOffsetX)/ m_RatioX) - square_size / 2,
        (((m_windowHeight - bOffsetY) / m_RatioY) - (ypos / m_RatioY)) - square_size / 2,
        square_size,
        texture,
        color[0], color[1], color[2], color[3]);
    
    Vertex target_vertices[1 * 4] = {0};
    memcpy(target_vertices, q.data(),  q.size() * sizeof(Vertex));

    m_VertexBuffer_Targets->Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(target_vertices), target_vertices);

}

void ChessGUI::OnRender()
{
    Renderer renderer;
    
    glm::mat4 model =  glm::scale(glm::mat4(1.0f), glm::vec3(m_RatioX, m_RatioY, 1.0f));   
    m_View =  glm::translate(glm::mat4(1.0f), glm::vec3(bOffsetX, 
                                                        bOffsetY, 1.0f));   
    m_Proj = glm::ortho(0.0f, (float)1920.0f, 0.0f, (float)1080.0f, -1.0f, 1.0f);

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

    iMouse.x = xpos;
    iMouse.y = ypos;

    iMouse.mode = action;
    iMouse.button = button;
     
    iMouse.updated = false;

    if (action == 1) iMouse.press_time = std::chrono::system_clock::now();

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
                float texture = flipped_board[i][j] > 0 ? (float)flipped_board[i][j]: (float)abs(flipped_board[i][j]) + 6.0;

                quad_count++;
                auto q = CreateQuad((square_size * j), (square_size * i), square_size, texture);

                memcpy(pieces_vertices + offset, q.data(),  q.size() * sizeof(Vertex));
                offset += q.size();
            }
        } 
    } 

    while (offset < m_NumPieceVertex)
    {
        quad_count++;
        auto q = CreateQuad((square_size * (-1)), (square_size * (-1)), square_size, 
                            0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

        memcpy(pieces_vertices + offset, q.data(),  q.size() * sizeof(Vertex));
        offset += q.size();
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


std::string coords_to_square(int row, int col){
    std::string s; s += (char)('a' + col); s += (char)('8' - (row));
    return s;
}
