#pragma once

#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
private:
    unsigned int m_RendererID;
public:
    VertexArray(); // Constructor
    ~VertexArray(); // Destructor

    void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
    unsigned int getRendererID() const { return m_RendererID; }

    void Bind() const;
    void Unbind() const;
};