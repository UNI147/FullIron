#include "Cube.h"
#include <cmath>

Cube::Cube() : rotationAngle(0.0f), rotationSpeed(30.0f) {
    setupBuffers();
}

Cube::~Cube() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
}

void Cube::setupBuffers() {
    // Куб 1x1x1, центрированный в (0,0,0)
    float size = 1.0f;
    
    float vertices[] = {
        // Позиции (x, y, z), Текстурные координаты (u, v)
        // Передняя грань
        -size, -size,  size,  0.0f, 0.0f,
         size, -size,  size,  1.0f, 0.0f,
         size,  size,  size,  1.0f, 1.0f,
        -size,  size,  size,  0.0f, 1.0f,
        
        // Задняя грань
        -size, -size, -size,  1.0f, 0.0f,
        -size,  size, -size,  1.0f, 1.0f,
         size,  size, -size,  0.0f, 1.0f,
         size, -size, -size,  0.0f, 0.0f,
        
        // Верхняя грань
        -size,  size, -size,  0.0f, 1.0f,
        -size,  size,  size,  0.0f, 0.0f,
         size,  size,  size,  1.0f, 0.0f,
         size,  size, -size,  1.0f, 1.0f,
        
        // Нижняя грань
        -size, -size, -size,  1.0f, 1.0f,
         size, -size, -size,  0.0f, 1.0f,
         size, -size,  size,  0.0f, 0.0f,
        -size, -size,  size,  1.0f, 0.0f,
        
        // Правая грань
         size, -size, -size,  1.0f, 0.0f,
         size,  size, -size,  1.0f, 1.0f,
         size,  size,  size,  0.0f, 1.0f,
         size, -size,  size,  0.0f, 0.0f,
        
        // Левая грань
        -size, -size, -size,  0.0f, 0.0f,
        -size, -size,  size,  1.0f, 0.0f,
        -size,  size,  size,  1.0f, 1.0f,
        -size,  size, -size,  0.0f, 1.0f
    };
    
    // Индексы для 6 граней (по 2 треугольника на грань)
    unsigned int indices[] = {
        // Передняя грань
        0, 1, 2, 2, 3, 0,
        // Задняя грань
        4, 5, 6, 6, 7, 4,
        // Верхняя грань
        8, 9, 10, 10, 11, 8,
        // Нижняя грань
        12, 13, 14, 14, 15, 12,
        // Правая грань
        16, 17, 18, 18, 19, 16,
        // Левая грань
        20, 21, 22, 22, 23, 20
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Позиции (атрибут 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Текстурные координаты (атрибут 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
                         (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Cube::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Cube::update(float deltaTime) {
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) {
        rotationAngle -= 360.0f;
    }
}