#ifndef ROTATOR_H
#define ROTATOR_H

#include "Component.h"
#include <glm/glm.hpp>

class Rotator : public Component {
public:
    Rotator(GameObject* owner);
    ~Rotator() override = default;
    
    void update(float deltaTime) override;
    
    void setRotationSpeed(const glm::vec3& speed) { m_rotationSpeed = speed; }
    void setRotationAxis(const glm::vec3& axis) { m_rotationAxis = axis; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    
    glm::vec3 getRotationSpeed() const { return m_rotationSpeed; }
    glm::vec3 getRotationAxis() const { return m_rotationAxis; }
    bool isEnabled() const { return m_enabled; }
    
private:
    glm::vec3 m_rotationSpeed = glm::vec3(0.0f, 30.0f, 0.0f);
    glm::vec3 m_rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
    bool m_enabled = true;
    float m_currentAngle = 0.0f;
};

#endif