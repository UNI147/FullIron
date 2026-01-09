#ifndef CUBERENDERER_H
#define CUBERENDERER_H

#include "GameObject/Component.h"
#include <memory>
#include <glm/glm.hpp>

// Forward declarations
class CLUTTexture;
class Shader;

class CubeRenderer : public Component {
public:
    CubeRenderer(GameObject* owner);
    ~CubeRenderer();
    
    void update(float deltaTime) override;
    void render() override;
    
    void setTexture(std::shared_ptr<CLUTTexture> texture);
    void setShader(std::shared_ptr<Shader> shader);
    void setColor(const glm::vec3& color);
    
    std::shared_ptr<CLUTTexture> getTexture() const { return m_texture; }
    std::shared_ptr<Shader> getShader() const { return m_shader; }
    const glm::vec3& getColor() const { return m_color; }
    
private:
    void setupBuffers();
    
    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;
    std::shared_ptr<CLUTTexture> m_texture;
    std::shared_ptr<Shader> m_shader;
    glm::vec3 m_color;
    bool m_visible;
};

#endif