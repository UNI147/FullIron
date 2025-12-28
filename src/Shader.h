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
    void setVec2(const std::string &name, float x, float y) const;
    
    GLuint getID() const { return ID; }

    void setBool(const std::string &name, bool value) const;
    
private:
    GLuint ID;
    
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif