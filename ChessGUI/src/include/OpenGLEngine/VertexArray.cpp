#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

#include <iostream>

VertexArray::VertexArray(){
    GLCall(glGenVertexArrays(1, &m_RendererID));
}
VertexArray::~VertexArray(){
    GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout){
    Bind();
    vb.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for (unsigned i=0; i < elements.size(); i++ ){

        const auto& element = elements[i];

        GLCall(glEnableVertexAttribArray(i)); // Enable the attribute (e.g. position)
        GLCall(glVertexAttribPointer(
            i, // Index of the attribute (e.g. position) 
            element.count, // Number of components per attribute (e.g. x, y)
            element.type, // Type of the components
            element.normalized, // Normalize the components (Floats are already normalized, but maybe we want to normalize colors)
            layout.GetStride(), // Stride (The ammount we need to go to get to the next vertex)
            reinterpret_cast<const void*>(static_cast<uintptr_t>(offset)) // Pointer (Location of the attribute inside the vertex)
            // (const void*)offset // This is the same as the line above, but issued a warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
        ));
        offset += element.count * VertexBufferLayoutElement::GetSizeOfType(element.type); 
    }
}

void VertexArray::Bind() const{
    GLCall(glBindVertexArray(m_RendererID));
}
void VertexArray::Unbind() const{
    GLCall(glBindVertexArray(0));
}