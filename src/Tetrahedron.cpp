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
    // Вершины тетраэдра (4 вершины)
    float vertices[] = {
        // Позиции (x, y, z)     // Текстурные координаты (u, v)
        // Вершина 0
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f,
        // Вершина 1
        0.5f, -0.5f, -0.5f,      1.0f, 0.0f,
        // Вершина 2
        0.0f, -0.5f, 0.5f,       0.5f, 0.5f,
        // Вершина 3 (верхняя)
        0.0f, 0.5f, 0.0f,        0.5f, 1.0f,
    };
    
    // Индексы для 4 треугольников (тетраэдр)
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
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0); // 4 треугольника * 3 вершины = 12
    glBindVertexArray(0);
}

void Tetrahedron::update(float deltaTime) {
    rotationAngle += rotationSpeed * deltaTime;
    if (rotationAngle > 360.0f) {
        rotationAngle -= 360.0f;
    }
}