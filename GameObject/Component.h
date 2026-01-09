#ifndef COMPONENT_H
#define COMPONENT_H

// Forward declarations
class GameObject;

class Component {
public:
    Component(GameObject* owner) : m_owner(owner) {}
    virtual ~Component() = default;
    
    virtual void update(float /*deltaTime*/) {}
    virtual void render() {}
    
    GameObject* getOwner() const { return m_owner; }
    
protected:
    GameObject* m_owner;
};

#endif