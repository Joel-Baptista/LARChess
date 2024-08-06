#pragma once

#include "include/OpenGLEngine/VertexBufferLayout.h"
#include "include/OpenGLEngine/VertexBuffer.h"
#include "include/OpenGLEngine/Texture.h"

#include <memory>
#include <array>
#include <GLFW/glfw3.h>

#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"

struct Vertex {
        float Position[3];
        float Color[4];
        float TexCoords[2];
        float TexID;
    };

class ChessGUI{
    public:
        ChessGUI(GLFWwindow* window);
        ~ChessGUI();

        void OnUpdate(float deltaTime);
        void OnRender();
        void OnImGuiRender();
        
        void set_board(std::array<std::array<int, 8>, 8> board);
        struct selected_square{
            int x = -1;
            int y = -1;
        };

    private:
        std::unique_ptr<VertexArray> m_VAO_Board;
        std::unique_ptr<IndexBuffer> m_IndexBuffer_Board;
        std::unique_ptr<VertexBuffer> m_VertexBuffer_Board;
        
        std::unique_ptr<VertexArray> m_VAO_Pieces;
        std::unique_ptr<IndexBuffer> m_IndexBuffer_Pieces;
        std::unique_ptr<VertexBuffer> m_VertexBuffer_Pieces;

        std::unique_ptr<VertexArray> m_VAO_Targets;
        std::unique_ptr<IndexBuffer> m_IndexBuffer_Targets;
        std::unique_ptr<VertexBuffer> m_VertexBuffer_Targets;

        std::unique_ptr<Shader> m_Shader;
        // std::unique_ptr<Texture> m_Texture1;
        // std::unique_ptr<Texture> m_Texture2;
        std::vector<std::unique_ptr<Texture>> m_Textures;

        int m_windowWidth;
        int m_windowHeight;
        GLFWwindow* m_Window;
        
        glm::mat4 m_Proj, m_View;
        glm::vec3 m_TranslationA, m_TranslationB;
        float m_QuadPosition[2] = {300.0f, 300.0f};
        float square_size = 100.0f;

        void get_board_indices(unsigned int* indices_board, int n_squares);
        void get_board_vertices(Vertex* vertices);
        void get_target_vertices(Vertex* vertices);

        std::array<std::array<int, 8>, 8> board = {
            {
            {-4, -2, -3, -5, -6, -3, -2, -4},
            {-1, -1, -1, -1, -1, -1, -1, -1},
            { 0,  0,  0,  0,  0,  0,  0,  0},
            { 0,  0,  0,  0,  0,  0,  0,  0},
            { 0,  0,  0,  0,  0,  0,  0,  0},
            { 0,  0,  0,  0,  0,  0,  0,  0},
            { 1,  1,  1,  1,  1,  1,  1,  1},
            { 4,  2,  3,  5,  6,  3,  2,  4}
            }    
        };

        void get_piece_vertices(Vertex* pieces_vertices);        
        enum class PieceType{
            W_PAWN = 1,
            W_KNIGHT = 2,
            W_BISHOP = 3,
            W_ROOK = 4,
            W_QUEEN = 5,
            W_KING = 6,
            B_PAWN = 7,
            B_KNIGHT = 8,
            B_BISHOP = 9,
            B_ROOK = 10,
            B_QUEEN = 11,
            B_KING = 12,
        };
};
    
static std::array<Vertex, 4> CreateQuad(
    float x, float y, float size, 
    float textureID, // x and y are the left bottom corner of the quad
    float r = 1.0f, float g = 0.5f, float b = 0.5f, float a = 1.5f) 
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

