#include "VertexBuffer.h"

#include "Renderer.h"
#include <iostream>

VertexBuffer::VertexBuffer(const void* data, unsigned int size, unsigned int draw_type){
    
    GLCall(glGenBuffers(1, &m_RendererID)); // Instantiate a buffer and store its ID  
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); // Select the buffer
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, draw_type)); // Fill the buffer with data
}

VertexBuffer::~VertexBuffer(){
    GLCall(glDeleteBuffers(1, &m_RendererID)); // Delete the buffer
}

void VertexBuffer::Bind() const{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); // Select the buffer
}

void VertexBuffer::Unbind() const{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); // Select the buffer
}