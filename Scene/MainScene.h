#ifndef MAINSCENE_H
#define MAINSCENE_H

#include "Scene.h"
#include <memory>

class MainScene : public Scene {
public:
    MainScene();
    ~MainScene() override;
    
    void load() override;
    void unload() override;
    void update(float deltaTime) override;
    void render() override;
    
private:
    // Для обратной совместимости
    void createDemoObjects();
    void setupCamera();
};

#endif