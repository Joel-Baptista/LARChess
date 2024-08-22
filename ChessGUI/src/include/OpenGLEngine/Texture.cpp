#include "Texture.h"
#include "../stb_image/stb_image.h"

#include <iostream>

Texture::Texture(const std::string& path) : m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0){ 

    stbi_set_flip_vertically_on_load(1); // Flip the image vertically (Open GL expects the origin to be at the bottom left)
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4); // Load the image (4 channels RGBA)

    // std::cout << "Texture: " << path << std::endl;
    // std::cout << "Width: " << m_Width << std::endl;
    // std::cout << "Height: " << m_Height << std::endl;
    // std::cout << "BPP: " << m_BPP << std::endl;

    GLCall(glGenTextures(1, &m_RendererID)); // Instantiate a texture and store its ID
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID)); // Select the texture

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)); // Set the min filter to linear
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)); // Set the mag filter to linear
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); 
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    // Fill the texture with the image data
    GLCall(glTexImage2D(
        GL_TEXTURE_2D,      // The target for the texture
        0,                  // The level of detail 
        GL_RGBA8,           // The internal format of the texture (How the texture is stored in the GPU) 
        m_Width, 
        m_Height, 
        0,                  // The border of the texture
        GL_RGBA,            // The format of the pixel data (How the data is stored in the CPU)
        GL_UNSIGNED_BYTE,   // The data type of the pixel data
        m_LocalBuffer       // The image data
    )); 

    GLCall(glBindTexture(GL_TEXTURE_2D, 0)); // Unselect the texture
    if (m_LocalBuffer) stbi_image_free(m_LocalBuffer); // Free the image data
}
Texture::~Texture(){
    GLCall(glDeleteTextures(1, &m_RendererID)); // Delete the texture
}   

void Texture::Bind(unsigned int slot) const{
    GLCall(glActiveTexture(GL_TEXTURE0 + slot)); // Select the texture slot
    GLCall(glBindTexture(GL_TEXTURE_2D, m_RendererID)); // Select the texture
}
void Texture::Unbind() const{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0)); // Unselect the texture
}
