#ifndef CUBE_H
#define CUBE_H

#include "glad.h"
#include <vector>

class Cube {
public:
    Cube();
    ~Cube();
    
    void draw() const;
    void update(float deltaTime);
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    float getRotationAngle() const { return rotationAngle; }
    
private:
    GLuint VAO, VBO, EBO;
    float rotationAngle;
    float rotationSpeed;
    
    void setupBuffers();
};

#endif