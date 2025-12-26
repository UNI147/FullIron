#ifndef SHADER_H
#define SHADER_H

#include <string>
#include "glad.h"

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    
    void use();
    void setMat4(const std::string &name, const float* value) const;
    void setInt(const std::string &name, int value) const;
    GLuint getID() const { return ID; }
    
private:
    GLuint ID;
    
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif