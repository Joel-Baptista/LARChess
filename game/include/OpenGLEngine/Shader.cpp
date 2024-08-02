#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

#include "Renderer.h"


Shader::Shader(const std::string& filepath)
    : m_FilePath(filepath), m_RendererID(0)
{
    ShaderProgramSource source = parse_shader(filepath);
    m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source){
    unsigned int id = glCreateShader(type); // Create a shader, that returns its ID
    const char* src = source.c_str(); // Get the pointer of the first character of the code
    glShaderSource(id, 1, &src, nullptr); // Set the source code of the shader
    glCompileShader(id); // Compile the shader

    // Error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); // Get the result of the compilation
    if (result == GL_FALSE){
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); // Get the length of the error message
        char message[length]; // Create a char array with the length of the error message
        glGetShaderInfoLog(id, length, &length, message); // Get the error message
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id); // Delete the shader
        return 0;
    }
    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader){
    /* This string are literally the source code for the shaders. We can have this strings hardcoded, or read them from files*/
    unsigned int program = glCreateProgram(); // Create a program, that returns its ID
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader); // Create a vertex shader, that returns its ID
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); // Create a vertex shader, that returns its ID

    glAttachShader(program, vs); // Attach the vertex shader to the program
    glAttachShader(program, fs); // Attach the fragment shader to the program
    glLinkProgram(program); // Link the program
    glValidateProgram(program); // Validate the program

    // Shaders are no longer needed after linking
    glDeleteShader(vs); // Delete the vertex shader
    glDeleteShader(fs); // Delete the fragment shader

    return program;
}

ShaderProgramSource Shader::parse_shader(const std::string& filepath){
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)){
        if (line.find("#shader") != std::string::npos){
            if (line.find("vertex") != std::string::npos){
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos){
                type = ShaderType::FRAGMENT; 
            }
        }else{
            ss[(int)type] << line << '\n';
        }
    }

    return {ss[0].str(), ss[1].str()};
}


unsigned int Shader::GetUniformLocation(const std::string& name){

    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()){ // Use cache to reduce the number of OpenGL calls
        return m_UniformLocationCache[name];
    }

    GLCall(int location = glGetUniformLocation(m_RendererID, name.c_str()));
    if (location == -1){
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }
    
    m_UniformLocationCache[name] = location;
    
    return location;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3){
    GLCall(glUniform4f(GetUniformLocation(name), v0, v1, v2, v3));
}

void Shader::SetUniform1f(const std::string& name, float value){
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform1i(const std::string& name, int value){
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1iv(const std::string& name, unsigned int size,GLint value[]){
    std::cout << size << std::endl;
    GLCall(glUniform1iv(GetUniformLocation(name), size, value));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix){
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}
