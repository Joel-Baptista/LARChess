#include "IndexBuffer.h"

#include "Renderer.h"

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) : m_count(count){
    ASSERT(sizeof(unsigned int) == sizeof(GLuint)); // Check if the size of unsigned int is the same as GLuint

    GLCall(glGenBuffers(1, &m_RendererID)); // Instantiate a buffer and store its ID  
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID)); // Select the buffer
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW)); // Fill the buffer with data
}

IndexBuffer::~IndexBuffer(){
    GLCall(glDeleteBuffers(1, &m_RendererID)); // Delete the buffer
}

void IndexBuffer::Bind() const{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID)); // Select the buffer
}

void IndexBuffer::Unbind() const{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); // Select the buffer
}