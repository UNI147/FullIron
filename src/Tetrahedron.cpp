#include "Tetrahedron.h"
#include <cmath>

Tetrahedron::Tetrahedron() : rotationAngle(0.0f), rotationSpeed(30.0f) {
    setupBuffers();
}

Tetrahedron::~Tetrahedron() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Tetrahedron::setupBuffers() {
    
    float vertices[] = {
        // ========== ОСНОВАНИЕ (нижняя грань) ==========
        // Вершина 0: задняя левая
        -0.5f, 0.0f, -0.288675f,     0.0f, 0.0f,  // UV: (0,0)
        // Вершина 1: задняя правая  
         0.5f, 0.0f, -0.288675f,     1.0f, 0.0f,  // UV: (1,0)
        // Вершина 2: передняя
         0.0f, 0.0f,  0.57735f,      0.5f, 1.0f,  // UV: (0.5,1)
        
        // ========== ЗАДНЯЯ ГРАНЬ (0-1-3) ==========
        // Вершина 0 (та же)
        -0.5f, 0.0f, -0.288675f,     0.0f, 0.0f,  // UV: (0,0)
        // Вершина 1 (та же)
         0.5f, 0.0f, -0.288675f,     1.0f, 0.0f,  // UV: (1,0)
        // Верхняя вершина 3
         0.0f, 1.0f,  0.0f,          0.5f, 1.0f,  // UV: (0.5,1)
        
        // ========== ПРАВАЯ ГРАНЬ (1-2-3) ==========
        // Вершина 1
         0.5f, 0.0f, -0.288675f,     0.0f, 0.0f,  // UV: (0,0)
        // Вершина 2
         0.0f, 0.0f,  0.57735f,      1.0f, 0.0f,  // UV: (1,0)
        // Верхняя вершина 3
         0.0f, 1.0f,  0.0f,          0.5f, 1.0f,  // UV: (0.5,1)
        
        // ========== ЛЕВАЯ ГРАНЬ (2-0-3) ==========
        // Вершина 2
         0.0f, 0.0f,  0.57735f,      0.0f, 0.0f,  // UV: (0,0)
        // Вершина 0
        -0.5f, 0.0f, -0.288675f,     1.0f, 0.0f,  // UV: (1,0)
        // Верхняя вершина 3
         0.0f, 1.0f,  0.0f,          0.5f, 1.0f,  // UV: (0.5,1)
    };
    
    // Индексы - каждая грань независимо
    unsigned int indices[] = {
        // Основание (0-1-2)
        0, 2, 1,
        // Задняя грань (3-4-5)
        3, 5, 4,
        // Правая грань (6-7-8)
        6, 8, 7,
        // Левая грань (9-10-11)
        9, 11, 10,
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

void Tetrahedron::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Tetrahedron::update(float deltaTime) {
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) {
        rotationAngle -= 360.0f;
    }
}