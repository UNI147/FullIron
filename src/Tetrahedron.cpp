#include "Tetrahedron.h"
#include <cmath>

Tetrahedron::Tetrahedron() : rotationAngle(0.0f), rotationSpeed(50.0f) {
    setupBuffers();
}

Tetrahedron::~Tetrahedron() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Tetrahedron::setupBuffers() {
    // Вершины тетраэдра (нормализованные координаты)
    float vertices[] = {
        // Позиции           // Текстурные координаты  // Нормали
        // Основание
        0.0f,  0.5f,  0.0f,  0.5f, 1.0f,              0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,              0.0f, 0.0f, 1.0f,
        0.5f, -0.5f,  0.0f,  1.0f, 0.0f,              0.0f, 0.0f, 1.0f,
        
        // Вершина
        0.0f,  0.0f,  0.8f,  0.5f, 0.5f,              0.0f, 1.0f, 0.0f,
    };
    
    // Индексы для треугольников
    unsigned int indices[] = {
        // Основание
        0, 1, 2,
        // Боковые грани
        0, 1, 3,
        1, 2, 3,
        2, 0, 3
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Позиции
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Текстурные координаты
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Нормали
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
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