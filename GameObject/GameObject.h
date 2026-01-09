#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>

// Forward declaration
class Component;

class GameObject {
public:
    GameObject(const std::string& name = "GameObject");
    virtual ~GameObject();
    
    void update(float deltaTime);
    void render();
    
    // Трансформация
    void setPosition(const glm::vec3& position);
    void setRotation(const glm::vec3& rotation);
    void setScale(const glm::vec3& scale);
    
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getRotation() const { return m_rotation; }
    glm::vec3 getScale() const { return m_scale; }
    
    glm::mat4 getModelMatrix() const;
    
    // Компоненты
    template<typename T, typename... Args>
    std::shared_ptr<T> addComponent(Args&&... args) {
        auto component = std::make_shared<T>(this, std::forward<Args>(args)...);
        m_components.push_back(component);
        return component;
    }
    
    template<typename T>
    std::shared_ptr<T> getComponent() {
        for (auto& component : m_components) {
            if (auto casted = std::dynamic_pointer_cast<T>(component)) {
                return casted;
            }
        }
        return nullptr;
    }
    
    // Иерархия
    void addChild(std::shared_ptr<GameObject> child);
    void removeChild(GameObject* child);
    
    GameObject* getParent() const { return m_parent; }
    const std::vector<std::shared_ptr<GameObject>>& getChildren() const { return m_children; }
    
    // Свойства
    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }
    
    void setActive(bool active) { m_active = active; }
    bool isActive() const { return m_active; }
    
private:
    std::string m_name;
    bool m_active;
    
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;
    
    GameObject* m_parent;
    std::vector<std::shared_ptr<GameObject>> m_children;
    std::vector<std::shared_ptr<Component>> m_components;
};

#endif