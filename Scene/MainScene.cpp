#include "MainScene.h"
#include "Core/ResourceManager.h"
#include "Components/Rotator.h"
#include "Components/Camera.h"
#include "Components/CubeRenderer.h"
#include "GameObject/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

MainScene::MainScene() : Scene("MainScene") {
}

MainScene::~MainScene() {
    unload();
}

void MainScene::load() {
    Scene::load();
    
    std::cout << "Loading MainScene resources..." << std::endl;
    
    // Загружаем ресурсы
    ResourceManager& rm = ResourceManager::getInstance();
    
    // Загружаем текстуру
    m_texture = rm.loadTexture("transformer", "resources/transformer.png");
    
    // Создаем объекты
    createDemoObjects();
    setupCamera();
    
    std::cout << "MainScene loaded successfully" << std::endl;
}

void MainScene::unload() {
    if (!isLoaded()) return;
    
    std::cout << "Unloading MainScene..." << std::endl;
    
    // Очищаем ресурсы
    m_texture.reset();
    m_cubeRenderer.reset();
    
    Scene::unload();
    
    std::cout << "MainScene unloaded" << std::endl;
}

void MainScene::update(float deltaTime) {
    Scene::update(deltaTime);
    
    // Обновляем вращение
    m_currentRotation += m_rotationSpeed * deltaTime;
    if (m_currentRotation > 360.0f) {
        m_currentRotation -= 360.0f;
    }
}

void MainScene::render() {
    Scene::render();
    
    // Здесь может быть дополнительная логика рендеринга
}

void MainScene::createDemoObjects() {
    std::cout << "Creating demo objects..." << std::endl;
    
    // Создаем основной игровой объект
    auto cubeObject = createGameObject("DemoCube");
    cubeObject->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Добавляем компонент вращения
    auto rotator = cubeObject->addComponent<Rotator>();
    if (rotator) {
        rotator->setRotationSpeed(glm::vec3(0.0f, 30.0f, 0.0f));
        rotator->setEnabled(true);
    }
    
    // Добавляем компонент рендеринга куба
    m_cubeRenderer = cubeObject->addComponent<CubeRenderer>();
    if (m_cubeRenderer && m_texture) {
        // Получаем шейдер из ResourceManager
        auto shader = ResourceManager::getInstance().getShader("clut");
        if (shader) {
            m_cubeRenderer->setShader(shader);
        }
        m_cubeRenderer->setTexture(m_texture);
        m_cubeRenderer->setColor(glm::vec3(1.0f, 1.0f, 1.0f));
    }
    
    std::cout << "Demo objects created" << std::endl;
}

void MainScene::setupCamera() {
    std::cout << "Setting up camera..." << std::endl;
    
    // Создаем объект камеры
    auto cameraObject = createGameObject("MainCamera");
    cameraObject->setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
    
    // Добавляем компонент камеры
    auto camera = cameraObject->addComponent<Camera>();
    if (camera) {
        camera->setPerspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
        camera->setAsMainCamera();
        setMainCamera(camera);
    }
    
    std::cout << "Camera setup complete" << std::endl;
}