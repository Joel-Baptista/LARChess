#pragma once

#include <GL/glew.h>

class VertexBuffer
{
private:
    unsigned int m_RendererID;
public:
    VertexBuffer(const void* data, unsigned int size, unsigned int draw_type = GL_STATIC_DRAW); // Constructor
    ~VertexBuffer(); // Destructor

    void Bind() const;
    void Unbind() const;
};