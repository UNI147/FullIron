#ifndef SCENECONFIG_H
#define SCENECONFIG_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>

struct GameObjectConfig {
    std::string name;
    std::string tag;
    std::string layer;
    
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    bool active = true;
    
    // Компоненты
    struct CameraComponent {
        bool enabled = false;
        float fov = 45.0f;
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        bool isMainCamera = false;
    };
    
    struct TransformComponent {
        bool enabled = true;
    };
    
    struct RotatorComponent {
        bool enabled = false;
        glm::vec3 rotationSpeed = glm::vec3(0.0f, 30.0f, 0.0f);
    };
    
    struct CubeRendererComponent {
        bool enabled = false;
        std::string texturePath;
        glm::vec3 color = glm::vec3(1.0f);
        float size = 1.0f;
        std::string shaderName = "default";
    };
    
    CameraComponent camera;
    TransformComponent transform;
    RotatorComponent rotator;
    CubeRendererComponent cubeRenderer;
    
    // Иерархия
    std::vector<std::string> children;
    std::string parent;
};

struct SceneConfig {
    std::string name;
    std::string filePath;
    
    // Настройки сцены
    glm::vec3 backgroundColor = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 ambientLight = glm::vec3(0.2f);
    glm::vec3 directionalLight = glm::vec3(1.0f);
    glm::vec3 lightDirection = glm::vec3(0.5f, -1.0f, 0.5f);
    
    // Объекты
    std::vector<GameObjectConfig> gameObjects;
    
    // Загрузка/сохранение
    bool loadFromFile(const std::string& filePath);
    bool saveToFile(const std::string& filePath);
    
    // Вспомогательные методы
    GameObjectConfig* findObject(const std::string& name);
    const GameObjectConfig* findObject(const std::string& name) const;
};

#endif