#pragma once

#include "include/OpenGLEngine/VertexBufferLayout.h"
#include "include/OpenGLEngine/VertexBuffer.h"
#include "include/OpenGLEngine/Texture.h"

#include <memory>
#include <GLFW/glfw3.h>

#include "include/glm/glm.hpp"
#include "include/glm/gtc/matrix_transform.hpp"


class ChessGUI{
    public:
        ChessGUI(GLFWwindow* window);
        ~ChessGUI();

        void OnUpdate(float deltaTime);
        void OnRender();
        void OnImGuiRender();


    private:
        std::unique_ptr<VertexArray> m_VAO_Board;
        std::unique_ptr<IndexBuffer> m_IndexBuffer_Board;
        std::unique_ptr<VertexBuffer> m_VertexBuffer_Board;
        
        std::unique_ptr<VertexArray> m_VAO_Pieces;
        std::unique_ptr<IndexBuffer> m_IndexBuffer_Pieces;
        std::unique_ptr<VertexBuffer> m_VertexBuffer_Pieces;

        std::unique_ptr<Shader> m_Shader;
        std::unique_ptr<Texture> m_Texture1;
        std::unique_ptr<Texture> m_Texture2;
        std::vector<std::unique_ptr<Texture>> m_Textures;

        int m_windowWidth;
        int m_windowHeight;
        GLFWwindow* m_Window;
        
        glm::mat4 m_Proj, m_View;
        glm::vec3 m_TranslationA, m_TranslationB;
        float m_QuadPosition[2] = {-50.0f, -50.0f};
};

