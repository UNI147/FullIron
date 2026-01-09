#include "CubeRenderer.h"
#include "Graphics/CLUTTexture.h"
#include "Graphics/Shader.h"
#include "GameObject/GameObject.h"
#include "Components/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

CubeRenderer::CubeRenderer(GameObject* owner) 
    : Component(owner), 
      m_VAO(0), 
      m_VBO(0), 
      m_EBO(0), 
      m_visible(true) {
    
    m_color.x = 1.0f;
    m_color.y = 1.0f;
    m_color.z = 1.0f;
    
    setupBuffers();
}

void CubeRenderer::setupBuffers() {
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
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    
    glBindVertexArray(m_VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
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

CubeRenderer::~CubeRenderer() {
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
}

void CubeRenderer::update(float /*deltaTime*/) {
    // CubeRenderer может не иметь логики обновления
}

void CubeRenderer::render() {
    if (!m_visible || !m_owner || !m_shader) return;
    
    // Получаем основную камеру
    Camera* mainCamera = Camera::getMainCamera();
    if (!mainCamera) {
        std::cerr << "No main camera found!" << std::endl;
        return;
    }
    
    // Используем шейдер
    m_shader->use();
    
    // Устанавливаем матрицы
    glm::mat4 model = m_owner->getModelMatrix();
    glm::mat4 view = mainCamera->getViewMatrix();
    glm::mat4 projection = mainCamera->getProjectionMatrix();
    glm::mat4 mvp = projection * view * model;
    
    m_shader->setMat4("mvp", glm::value_ptr(mvp));
    m_shader->setMat4("model", glm::value_ptr(model));
    m_shader->setMat4("view", glm::value_ptr(view));
    m_shader->setMat4("projection", glm::value_ptr(projection));
    
    // Устанавливаем разрешение (простые константы для теста)
    m_shader->setVec2("resolution", 320.0f, 240.0f);
    
    // Устанавливаем позицию камеры
    glm::vec3 cameraPos = mainCamera->getOwner()->getPosition();
    m_shader->setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    
    // Привязываем текстуру, если есть
    if (m_texture) {
        m_texture->bind(0);
        m_shader->setInt("indexTexture", 0);
        m_shader->setInt("paletteTexture", 1);
    }
    
    // Устанавливаем цвет
    m_shader->setVec3("color", m_color.x, m_color.y, m_color.z);
    
    // Рендерим куб
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CubeRenderer::setTexture(std::shared_ptr<CLUTTexture> texture) {
    m_texture = texture;
}

void CubeRenderer::setShader(std::shared_ptr<Shader> shader) {
    m_shader = shader;
}

void CubeRenderer::setColor(const glm::vec3& color) {
    m_color = color;
}