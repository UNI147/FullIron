#include "Rotator.h"
#include "GameObject/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>

Rotator::Rotator(GameObject* owner) 
    : Component(owner) {
}

void Rotator::update(float deltaTime) {
    if (!m_enabled || !m_owner) return;
    
    // Обновляем угол вращения
    m_currentAngle += m_rotationSpeed.y * deltaTime;
    if (m_currentAngle > 360.0f) {
        m_currentAngle -= 360.0f;
    }
    
    // Получаем текущее вращение
    glm::vec3 rotation = m_owner->getRotation();
    
    // Применяем вращение
    rotation.y += m_rotationSpeed.y * deltaTime;
    
    // Нормализуем угол
    if (rotation.y > 360.0f) {
        rotation.y -= 360.0f;
    }
    
    // Устанавливаем новое вращение
    m_owner->setRotation(rotation);
}