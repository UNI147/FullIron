#include "SceneConfig.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "json/json.h"

bool SceneConfig::loadFromFile(const std::string& filePath) {
    this->filePath = filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open scene file: " << filePath << std::endl;
        return false;
    }
    
    try {
        Json::Value root;
        file >> root;
        
        // Загружаем основные параметры сцены
        name = root.get("name", "Unnamed Scene").asString();
        
        if (root.isMember("backgroundColor")) {
            const auto& bg = root["backgroundColor"];
            backgroundColor = glm::vec3(
                bg[0].asFloat(),
                bg[1].asFloat(),
                bg[2].asFloat()
            );
        }
        
        if (root.isMember("ambientLight")) {
            const auto& al = root["ambientLight"];
            ambientLight = glm::vec3(
                al[0].asFloat(),
                al[1].asFloat(),
                al[2].asFloat()
            );
        }
        
        // Загружаем объекты
        if (root.isMember("gameObjects") && root["gameObjects"].isArray()) {
            const auto& objects = root["gameObjects"];
            gameObjects.clear();
            
            for (const auto& objJson : objects) {
                GameObjectConfig obj;
                obj.name = objJson.get("name", "GameObject").asString();
                obj.tag = objJson.get("tag", "").asString();
                
                // Загрузка трансформации
                if (objJson.isMember("transform")) {
                    const auto& transform = objJson["transform"];
                    const auto& pos = transform["position"];
                    const auto& rot = transform["rotation"];
                    const auto& scale = transform["scale"];
                    
                    obj.position = glm::vec3(
                        pos[0].asFloat(),
                        pos[1].asFloat(),
                        pos[2].asFloat()
                    );
                    
                    obj.rotation = glm::vec3(
                        rot[0].asFloat(),
                        rot[1].asFloat(),
                        rot[2].asFloat()
                    );
                    
                    obj.scale = glm::vec3(
                        scale[0].asFloat(),
                        scale[1].asFloat(),
                        scale[2].asFloat()
                    );
                }
                
                // Загрузка компонентов
                if (objJson.isMember("components")) {
                    const auto& comps = objJson["components"];
                    
                    // Camera
                    if (comps.isMember("camera")) {
                        const auto& cam = comps["camera"];
                        obj.camera.enabled = cam.get("enabled", false).asBool();
                        if (obj.camera.enabled) {
                            obj.camera.fov = cam.get("fov", 45.0f).asFloat();
                            obj.camera.nearPlane = cam.get("nearPlane", 0.1f).asFloat();
                            obj.camera.farPlane = cam.get("farPlane", 100.0f).asFloat();
                            obj.camera.isMainCamera = cam.get("isMainCamera", false).asBool();
                        }
                    }
                    
                    // Rotator
                    if (comps.isMember("rotator")) {
                        const auto& rot = comps["rotator"];
                        obj.rotator.enabled = rot.get("enabled", false).asBool();
                        if (obj.rotator.enabled) {
                            const auto& speed = rot["rotationSpeed"];
                            obj.rotator.rotationSpeed = glm::vec3(
                                speed[0].asFloat(),
                                speed[1].asFloat(),
                                speed[2].asFloat()
                            );
                        }
                    }
                    
                    // CubeRenderer
                    if (comps.isMember("cubeRenderer")) {
                        const auto& cube = comps["cubeRenderer"];
                        obj.cubeRenderer.enabled = cube.get("enabled", false).asBool();
                        if (obj.cubeRenderer.enabled) {
                            obj.cubeRenderer.texturePath = cube.get("texturePath", "").asString();
                            obj.cubeRenderer.shaderName = cube.get("shader", "default").asString();
                            
                            const auto& color = cube["color"];
                            obj.cubeRenderer.color = glm::vec3(
                                color[0].asFloat(),
                                color[1].asFloat(),
                                color[2].asFloat()
                            );
                            
                            obj.cubeRenderer.size = cube.get("size", 1.0f).asFloat();
                        }
                    }
                }
                
                // Иерархия
                if (objJson.isMember("parent")) {
                    obj.parent = objJson["parent"].asString();
                }
                
                if (objJson.isMember("children") && objJson["children"].isArray()) {
                    for (const auto& child : objJson["children"]) {
                        obj.children.push_back(child.asString());
                    }
                }
                
                gameObjects.push_back(obj);
            }
        }
        
        std::cout << "Scene loaded: " << name << " (" << gameObjects.size() << " objects)" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse scene file: " << e.what() << std::endl;
        return false;
    }
}

GameObjectConfig* SceneConfig::findObject(const std::string& name) {
    for (auto& obj : gameObjects) {
        if (obj.name == name) {
            return &obj;
        }
    }
    return nullptr;
}

const GameObjectConfig* SceneConfig::findObject(const std::string& name) const {
    for (const auto& obj : gameObjects) {
        if (obj.name == name) {
            return &obj;
        }
    }
    return nullptr;
}