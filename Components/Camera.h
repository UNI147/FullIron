#ifndef CAMERA_H
#define CAMERA_H

#include "GameObject/Component.h"
#include <glm/glm.hpp>
#include "GameObject/GameObject.h"

class Camera : public Component {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };
    
    Camera(GameObject* owner);
    ~Camera() override = default;
    
    void update(float /*deltaTime*/) override {}
    
    // Проекция
    void setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane);
    void setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    
    // Вид
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // Вспомогательный метод для получения позиции
    glm::vec3 getOwnerPosition() const;
    
    // Свойства камеры
    void setFOV(float fov);
    void setNearPlane(float nearPlane);
    void setFarPlane(float farPlane);
    void setAspectRatio(float aspectRatio);
    
    float getFOV() const { return m_fov; }
    float getNearPlane() const { return m_nearPlane; }
    float getFarPlane() const { return m_farPlane; }
    float getAspectRatio() const { return m_aspectRatio; }
    
    // Контроллер камеры
    void lookAt(const glm::vec3& target);
    void move(const glm::vec3& offset);
    
    // Основная камера
    static Camera* getMainCamera() { return s_mainCamera; }
    void setAsMainCamera();
    
private:
    ProjectionType m_projectionType = ProjectionType::Perspective;
    float m_fov = 45.0f;
    float m_aspectRatio = 4.0f / 3.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 100.0f;
    
    // Ортографические параметры
    float m_orthoLeft = -1.0f;
    float m_orthoRight = 1.0f;
    float m_orthoBottom = -1.0f;
    float m_orthoTop = 1.0f;
    
    static Camera* s_mainCamera;
};

#endif