#pragma once

class IndexBuffer
{
private:
    unsigned int m_RendererID;
    unsigned int m_count;
public:
    IndexBuffer(const unsigned int* data, unsigned int count); // Count is the number of element (not bytes like in size)
    ~IndexBuffer(); // Destructor

    void Bind() const;
    void Unbind() const;

    unsigned int getRendererID() const { return m_RendererID; } // Getter for m_RendererID

    inline unsigned int GetCount() const { return m_count; } // Getter for m_count
};