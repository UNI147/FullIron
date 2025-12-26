#ifndef TETRAHEDRON_H
#define TETRAHEDRON_H

#include "glad.h"
#include <vector>

class Tetrahedron {
public:
    Tetrahedron();
    ~Tetrahedron();
    
    void draw() const;
    void update(float deltaTime);
    void setRotationSpeed(float speed) { rotationSpeed = speed; }
    
private:
    GLuint VAO, VBO, EBO;
    float rotationAngle;
    float rotationSpeed;
    
    void setupBuffers();
};

#endif