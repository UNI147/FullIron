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
    void setMat4GLM(const std::string &name, const void* glmMatrix) const;
    void setInt(const std::string &name, int value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setFloat(const std::string &name, float value) const;
    void setBool(const std::string &name, bool value) const;
    
    GLuint getID() const { return ID; }
    
private:
    GLuint ID;
    
    void checkCompileErrors(GLuint shader, std::string type);
};

#endif