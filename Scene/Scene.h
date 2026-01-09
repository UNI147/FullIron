#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

// Forward declarations
class GameObject;
class Camera;

class Scene {
public:
    Scene(const std::string& name);
    virtual ~Scene();
    
    virtual void load();
    virtual void unload();
    virtual void update(float deltaTime);
    virtual void render();
    
    // Управление объектами
    std::shared_ptr<GameObject> createGameObject(const std::string& name = "GameObject");
    void addGameObject(std::shared_ptr<GameObject> obj);
    void removeGameObject(GameObject* obj);
    
    // Поиск объектов
    std::shared_ptr<GameObject> findGameObject(const std::string& name);
    std::vector<std::shared_ptr<GameObject>> findGameObjectsWithTag(const std::string& tag);
    
    // Камера
    void setMainCamera(std::shared_ptr<Camera> camera);
    std::shared_ptr<Camera> getMainCamera() const { return m_mainCamera; }
    
    // Свойства
    const std::string& getName() const { return m_name; }
    bool isLoaded() const { return m_loaded; }
    
protected:
    std::string m_name;
    bool m_loaded;
    
    std::vector<std::shared_ptr<GameObject>> m_gameObjects;
    std::shared_ptr<Camera> m_mainCamera;
    
    virtual void onLoad() {}
    virtual void onUnload() {}
    virtual void onUpdate(float /*deltaTime*/) {}
    virtual void onRender() {}
};

#endif