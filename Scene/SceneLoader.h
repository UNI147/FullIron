#ifndef SCENELOADER_H
#define SCENELOADER_H

#include "ConfigParser.h"
#include "Scene.h"
#include <memory>
#include <unordered_map>

class SceneLoader {
public:
    static SceneLoader& getInstance();
    
    std::shared_ptr<Scene> loadScene(const std::string& sceneName);
    void createSceneObjects(Scene* scene, const ConfigParser::SceneConfig& config);
    
private:
    SceneLoader() = default;
    
    std::shared_ptr<GameObject> createGameObjectFromConfig(
        Scene* scene,
        const ConfigParser::GameObjectConfig& config
    );
};
#endif