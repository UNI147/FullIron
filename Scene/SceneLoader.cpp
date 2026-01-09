#include "SceneLoader.h"
#include "Core/ResourceManager.h"
#include "Components/Camera.h"
#include "Components/Rotator.h"
#include "Components/CubeRenderer.h"
#include <iostream>

SceneLoader& SceneLoader::getInstance() {
    static SceneLoader instance;
    return instance;
}

std::shared_ptr<Scene> SceneLoader::loadScene(const std::string& sceneName) {
    std::string filePath = "scenes/" + sceneName + ".scene";
    
    ConfigParser::SceneConfig config;
    config.name = sceneName;
    
    if (!ConfigParser::parseSceneFile(filePath, config)) {
        std::cerr << "Failed to load scene: " << sceneName << std::endl;
        return nullptr;
    }
    
    auto scene = std::make_shared<Scene>(config.name);
    createSceneObjects(scene.get(), config);
    
    return scene;
}

void SceneLoader::createSceneObjects(Scene* scene, const ConfigParser::SceneConfig& config) {
    std::unordered_map<std::string, std::shared_ptr<GameObject>> createdObjects;
    
    // Сначала создаем все объекты
    for (const auto& objConfig : config.objects) {
        auto obj = createGameObjectFromConfig(scene, objConfig);
        if (obj) {
            createdObjects[objConfig.name] = obj;
        }
    }
    
    // Затем устанавливаем иерархию
    for (const auto& objConfig : config.objects) {
        if (!objConfig.parent.empty()) {
            auto parentIt = createdObjects.find(objConfig.parent);
            auto childIt = createdObjects.find(objConfig.name);
            
            if (parentIt != createdObjects.end() && childIt != createdObjects.end()) {
                parentIt->second->addChild(childIt->second);
            }
        }
        
        // Также устанавливаем детей из списка children
        for (const auto& childName : objConfig.children) {
            auto parentIt = createdObjects.find(objConfig.name);
            auto childIt = createdObjects.find(childName);
            
            if (parentIt != createdObjects.end() && childIt != createdObjects.end()) {
                parentIt->second->addChild(childIt->second);
            }
        }
    }
}

std::shared_ptr<GameObject> SceneLoader::createGameObjectFromConfig(
    Scene* scene,
    const ConfigParser::GameObjectConfig& config) {
    
    auto obj = scene->createGameObject(config.name);
    obj->setPosition(config.position);
    obj->setRotation(config.rotation);
    obj->setScale(config.scale);
    
    // Создаем компоненты в зависимости от типа
    if (config.type == "camera" || config.isMainCamera) {
        auto camera = obj->addComponent<Camera>();
        camera->setPerspective(
            config.fov,
            4.0f / 3.0f,
            config.nearPlane,
            config.farPlane
        );
        if (config.isMainCamera) {
            camera->setAsMainCamera();
            scene->setMainCamera(camera);
        }
    }
    else if (config.type == "cube" || config.rotationSpeed > 0 || !config.texture.empty()) {
        // Добавляем вращение если есть
        if (config.rotationSpeed > 0) {
            auto rotator = obj->addComponent<Rotator>();
            rotator->setRotationSpeed(glm::vec3(0.0f, config.rotationSpeed, 0.0f));
            rotator->setEnabled(true);
        }
        
        // Добавляем рендерер куба
        auto cubeRenderer = obj->addComponent<CubeRenderer>();
        
        // Загружаем текстуру если указана
        if (!config.texture.empty()) {
            auto texture = ResourceManager::getInstance().loadTexture(
                config.name + "_tex",
                config.texture
            );
            cubeRenderer->setTexture(texture);
        }
        
        // Устанавливаем шейдер и цвет
        auto shader = ResourceManager::getInstance().getShader("clut");
        if (shader) {
            cubeRenderer->setShader(shader);
        }
        cubeRenderer->setColor(config.color);
    }
    
    return obj;
}