#include "GameObject.h"
#include "Component.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

GameObject::GameObject(const std::string& name) 
    : m_name(name), m_active(true), 
      m_position(0.0f), m_rotation(0.0f), m_scale(1.0f),
      m_parent(nullptr) {
}

GameObject::~GameObject() {
    // Удаляем себя из родителя
    if (m_parent) {
        m_parent->removeChild(this);
    }
    
    // Удаляем детей
    m_children.clear();
    m_components.clear();
}

void GameObject::update(float deltaTime) {
    if (!m_active) return;
    
    // Обновляем все компоненты
    for (auto& component : m_components) {
        component->update(deltaTime);
    }
    
    // Обновляем детей
    for (auto& child : m_children) {
        child->update(deltaTime);
    }
}

void GameObject::render() {
    if (!m_active) return;
    
    // Рендерим все компоненты рендеринга
    for (auto& component : m_components) {
        component->render();
    }
    
    // Рендерим детей
    for (auto& child : m_children) {
        child->render();
    }
}

void GameObject::setPosition(const glm::vec3& position) {
    m_position = position;
}

void GameObject::setRotation(const glm::vec3& rotation) {
    m_rotation = rotation;
}

void GameObject::setScale(const glm::vec3& scale) {
    m_scale = scale;
}

glm::mat4 GameObject::getModelMatrix() const {
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_position);
    
    // Создаем матрицу вращения из углов Эйлера
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(m_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(m_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, glm::radians(m_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), m_scale);
    
    glm::mat4 model = translation * rotation * scaleMat;
    
    // Применяем родительскую трансформацию
    if (m_parent) {
        model = m_parent->getModelMatrix() * model;
    }
    
    return model;
}

void GameObject::addChild(std::shared_ptr<GameObject> child) {
    if (!child || child.get() == this) return;
    
    // Удаляем из предыдущего родителя
    if (child->m_parent && child->m_parent != this) {
        child->m_parent->removeChild(child.get());
    }
    
    child->m_parent = this;
    m_children.push_back(child);
}

void GameObject::removeChild(GameObject* child) {
    if (!child) return;
    
    auto it = std::remove_if(m_children.begin(), m_children.end(),
        [child](const std::shared_ptr<GameObject>& obj) {
            return obj.get() == child;
        });
    
    if (it != m_children.end()) {
        (*it)->m_parent = nullptr;
        m_children.erase(it, m_children.end());
    }
}