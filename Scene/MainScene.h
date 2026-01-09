#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "Scene.h"
#include <memory>

// Forward declarations
class CLUTTexture;
class CubeRenderer;
class Camera;

class MainScene : public Scene {
public:
    MainScene();
    ~MainScene() override;
    
    void load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;
    
private:
    std::shared_ptr<CLUTTexture> m_texture;
    std::shared_ptr<CubeRenderer> m_cubeRenderer;
    float m_rotationSpeed = 30.0f;
    float m_currentRotation = 0.0f;
    
    void createDemoObjects();
    void setupCamera();
};

#endif