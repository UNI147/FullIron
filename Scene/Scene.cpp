#include "Scene.h"
#include "Components/Camera.h"
#include "GameObject/GameObject.h"
#include <algorithm>
#include <iostream>

Scene::Scene(const std::string& name) 
    : m_name(name), m_loaded(false) {
}

Scene::~Scene() {
    unload();
}

void Scene::load() {
    if (m_loaded) return;
    
    std::cout << "Loading scene: " << m_name << std::endl;
    
    // Загружаем пользовательскую логику
    onLoad();
    
    m_loaded = true;
    std::cout << "Scene loaded: " << m_name << std::endl;
}

void Scene::unload() {
    if (!m_loaded) return;
    
    std::cout << "Unloading scene: " << m_name << std::endl;
    
    // Вызываем пользовательскую логику выгрузки
    onUnload();
    
    // Очищаем игровые объекты
    m_gameObjects.clear();
    m_mainCamera.reset();
    
    m_loaded = false;
    std::cout << "Scene unloaded: " << m_name << std::endl;
}

void Scene::update(float deltaTime) {
    if (!m_loaded) return;
    
    // Обновляем пользовательскую логику
    onUpdate(deltaTime);
    
    // Обновляем все игровые объекты
    for (auto& obj : m_gameObjects) {
        if (obj->isActive()) {
            obj->update(deltaTime);
        }
    }
}

void Scene::render() {
    if (!m_loaded) return;
    
    // Рендерим пользовательскую логику
    onRender();
    
    // Рендерим все активные объекты
    for (auto& obj : m_gameObjects) {
        if (obj->isActive()) {
            obj->render();
        }
    }
}

std::shared_ptr<GameObject> Scene::createGameObject(const std::string& name) {
    auto obj = std::make_shared<GameObject>(name);
    m_gameObjects.push_back(obj);
    return obj;
}

void Scene::addGameObject(std::shared_ptr<GameObject> obj) {
    if (!obj) return;
    
    m_gameObjects.push_back(obj);
}

void Scene::removeGameObject(GameObject* obj) {
    if (!obj) return;
    
    auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
        [obj](const std::shared_ptr<GameObject>& o) {
            return o.get() == obj;
        });
    
    if (it != m_gameObjects.end()) {
        m_gameObjects.erase(it, m_gameObjects.end());
    }
}

std::shared_ptr<GameObject> Scene::findGameObject(const std::string& name) {
    for (auto& obj : m_gameObjects) {
        if (obj->getName() == name) {
            return obj;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<GameObject>> Scene::findGameObjectsWithTag(const std::string& /*tag*/) {
    std::vector<std::shared_ptr<GameObject>> result;
    // TODO: Добавить систему тегов к GameObject
    return result;
}

void Scene::setMainCamera(std::shared_ptr<Camera> camera) {
    m_mainCamera = camera;
    if (camera) {
        camera->setAsMainCamera();
    }
}