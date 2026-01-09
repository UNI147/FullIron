#include "SceneManager.h"
#include <iostream>

SceneManager& SceneManager::getInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::registerScene(const std::string& name, std::shared_ptr<Scene> scene) {
    auto it = m_scenes.find(name);
    if (it != m_scenes.end()) {
        std::cerr << "Scene '" << name << "' already registered" << std::endl;
        return;
    }
    
    m_scenes[name] = scene;
    std::cout << "Scene registered: " << name << std::endl;
}

void SceneManager::loadScene(const std::string& name) {
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) {
        std::cerr << "Scene not found: " << name << std::endl;
        return;
    }
    
    // Выгружаем текущую сцену
    if (m_currentScene) {
        unloadCurrentScene();
    }
    
    // Загружаем новую сцену
    m_currentScene = it->second;
    m_currentScene->load();
    
    std::cout << "Scene loaded: " << name << std::endl;
}

void SceneManager::unloadCurrentScene() {
    if (!m_currentScene) return;
    
    std::string sceneName = m_currentScene->getName();
    m_currentScene->unload();
    m_currentScene.reset();
    
    std::cout << "Scene unloaded: " << sceneName << std::endl;
}

void SceneManager::update(float deltaTime) {
    if (m_currentScene && m_currentScene->isLoaded()) {
        m_currentScene->update(deltaTime);
    }
}

void SceneManager::render() {
    if (m_currentScene && m_currentScene->isLoaded()) {
        m_currentScene->render();
    }
}

void SceneManager::cleanup() {
    unloadCurrentScene();
    m_scenes.clear();
    std::cout << "SceneManager cleaned up" << std::endl;
}