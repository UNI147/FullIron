#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::cout << "Loading shaders from: " << vertexPath << " and " << fragmentPath << std::endl;
    
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    bool usingFallback = false;
    
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        
        std::cout << "Successfully loaded shader files" << std::endl;
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        usingFallback = true;
        
        // Создаем простые шейдеры по умолчанию
        vertexCode = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;
uniform mat4 mvp;
void main() {
    gl_Position = mvp * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
        )";
        
        fragmentCode = R"(
#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
void main() {
    FragColor = texture(texture1, TexCoord);
}
        )";
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    if (usingFallback) {
        std::cout << "Using fallback built-in shaders" << std::endl;
    }
    
    GLuint vertex, fragment;
    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(ID);
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setMat4(const std::string &name, const float* value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, value);
    }
}

// Реализация setMat4GLM
void Shader::setMat4GLM(const std::string &name, const void* glmMatrix) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        // Преобразуем void* в указатель на float
        const float* matrixData = reinterpret_cast<const float*>(glmMatrix);
        glUniformMatrix4fv(location, 1, GL_FALSE, matrixData);
    }
}

void Shader::setInt(const std::string &name, int value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniform2f(location, x, y);
    }
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniform3f(location, x, y, z);
    }
}

void Shader::setFloat(const std::string &name, float value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Shader::setBool(const std::string &name, bool value) const {
    GLint location = glGetUniformLocation(ID, name.c_str());
    if (location != -1) {
        glUniform1i(location, static_cast<int>(value));
    }
}

void Shader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << infoLog << std::endl;
        }
    }
}