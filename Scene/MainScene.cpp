#include "MainScene.h"
#include "Core/ResourceManager.h"
#include "Components/Rotator.h"
#include "Components/Camera.h"
#include "Components/CubeRenderer.h"
#include "SceneLoader.h"
#include "ConfigParser.h"
#include <iostream>

MainScene::MainScene() : Scene("MainScene") {
}

MainScene::~MainScene() {
    unload();
}

void MainScene::load() {
    Scene::load();
    
    std::cout << "Loading MainScene..." << std::endl;
    
    // Пытаемся загрузить сцену из конфига
    auto& sceneLoader = SceneLoader::getInstance();
    
    // Загружаем конфигурацию
    ConfigParser::SceneConfig config;
    config.name = "MainScene";
    
    // Пытаемся загрузить из файла
    std::string filePath = "scenes/main_scene.scene";
    
    if (ConfigParser::parseSceneFile(filePath, config)) {
        // Успешно загружено из файла
        std::cout << "MainScene loaded from configuration file" << std::endl;
        sceneLoader.createSceneObjects(this, config);
    } else {
        // Не удалось загрузить из файла - используем fallback
        std::cout << "Cannot load scene from config, using fallback..." << std::endl;
        createDemoObjects();
        setupCamera();
    }
    
    std::cout << "MainScene loaded successfully" << std::endl;
}

void MainScene::unload() {
    if (!isLoaded()) return;
    
    std::cout << "Unloading MainScene..." << std::endl;
    Scene::unload();
    std::cout << "MainScene unloaded" << std::endl;
}

void MainScene::update(float deltaTime) {
    Scene::update(deltaTime);
}

void MainScene::render() {
    Scene::render();
}

// Fallback методы для обратной совместимости
void MainScene::createDemoObjects() {
    std::cout << "Creating fallback demo objects..." << std::endl;
    
    auto cubeObject = createGameObject("DemoCube");
    cubeObject->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    
    auto rotator = cubeObject->addComponent<Rotator>();
    if (rotator) {
        rotator->setRotationSpeed(glm::vec3(0.0f, 30.0f, 0.0f));
        rotator->setEnabled(true);
    }
    
    auto cubeRenderer = cubeObject->addComponent<CubeRenderer>();
    auto shader = ResourceManager::getInstance().getShader("clut");
    if (shader) {
        cubeRenderer->setShader(shader);
    }
    
    auto texture = ResourceManager::getInstance().loadTexture(
        "transformer", 
        "resources/transformer.png"
    );
    if (texture) {
        cubeRenderer->setTexture(texture);
    }
    
    cubeRenderer->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
}

void MainScene::setupCamera() {
    std::cout << "Setting up fallback camera..." << std::endl;
    
    auto cameraObject = createGameObject("MainCamera");
    cameraObject->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    
    auto camera = cameraObject->addComponent<Camera>();
    if (camera) {
        camera->setPerspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        camera->setAsMainCamera();
        setMainCamera(camera);
    }
}