#include "Camera.h"
#include "GameObject/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Camera* Camera::s_mainCamera = nullptr;

Camera::Camera(GameObject* owner) 
    : Component(owner) {
}

void Camera::setPerspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    m_projectionType = ProjectionType::Perspective;
    m_fov = fov;
    m_aspectRatio = aspectRatio;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void Camera::setOrthographic(float left, float right, float bottom, float top, 
                            float nearPlane, float farPlane) {
    m_projectionType = ProjectionType::Orthographic;
    m_orthoLeft = left;
    m_orthoRight = right;
    m_orthoBottom = bottom;
    m_orthoTop = top;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

glm::vec3 Camera::getOwnerPosition() const {
    if (!m_owner) return glm::vec3(0.0f);
    return m_owner->getPosition();
}

glm::mat4 Camera::getViewMatrix() const {
    if (!m_owner) return glm::mat4(1.0f);
    
    glm::vec3 position = getOwnerPosition();
    glm::vec3 rotation = m_owner->getRotation();
    
    // Создаем матрицу вращения
    glm::mat4 rotationMat = glm::mat4(1.0f);
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    rotationMat = glm::rotate(rotationMat, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    
    // Получаем направления взгляда
    glm::vec3 forward = glm::vec3(rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
    glm::vec3 up = glm::vec3(rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    
    return glm::lookAt(position, position + forward, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (m_projectionType == ProjectionType::Perspective) {
        return glm::perspective(glm::radians(m_fov), m_aspectRatio, 
                               m_nearPlane, m_farPlane);
    } else {
        return glm::ortho(m_orthoLeft, m_orthoRight, m_orthoBottom, 
                         m_orthoTop, m_nearPlane, m_farPlane);
    }
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

void Camera::setFOV(float fov) {
    m_fov = fov;
}

void Camera::setNearPlane(float nearPlane) {
    m_nearPlane = nearPlane;
}

void Camera::setFarPlane(float farPlane) {
    m_farPlane = farPlane;
}

void Camera::setAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
}

void Camera::lookAt(const glm::vec3& target) {
    if (!m_owner) return;
    
    glm::vec3 position = getOwnerPosition();
    glm::vec3 direction = glm::normalize(target - position);
    
    // Вычисляем углы Эйлера из направления
    glm::vec3 rotation(0.0f);
    rotation.y = glm::degrees(atan2f(direction.x, direction.z));
    rotation.x = glm::degrees(asinf(-direction.y));
    
    m_owner->setRotation(rotation);
}

void Camera::move(const glm::vec3& offset) {
    if (!m_owner) return;
    
    glm::vec3 position = getOwnerPosition();
    m_owner->setPosition(position + offset);
}

void Camera::setAsMainCamera() {
    s_mainCamera = this;
    std::cout << "Camera set as main camera" << std::endl;
}