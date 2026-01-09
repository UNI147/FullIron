#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <memory>
#include <unordered_map>
#include <string>
#include "Scene.h"

class SceneManager {
public:
    static SceneManager& getInstance();
    
    void registerScene(const std::string& name, std::shared_ptr<Scene> scene);
    void loadScene(const std::string& name);
    void unloadCurrentScene();
    
    void update(float deltaTime);
    void render();
    void cleanup();
    
    Scene* getCurrentScene() const { return m_currentScene.get(); }
    
private:
    SceneManager() = default;
    ~SceneManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
    std::shared_ptr<Scene> m_currentScene;
};

#endif