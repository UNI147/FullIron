#ifndef ENGINE_H
#define ENGINE_H

#include <memory>
#include <string>
#include <chrono>
#include <windows.h>
#include "Graphics/LowResFramebuffer.h"
#include "Scene/SceneManager.h"
#include "Graphics/GraphicsConfig.h"

class Engine {
public:
    Engine(HWND hwnd, HDC hdc, int internalWidth, int internalHeight);
    ~Engine();
    
    void update(float deltaTime);
    void render();
    
    void setScreenSize(int width, int height);
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void setPaused(bool paused);
    
    bool isFullscreen() const { return m_fullscreen; }
    bool isPaused() const { return m_paused; }
    
    float getTime() const { return m_time; }
    
//    static Engine* getInstance() { return s_instance; }
    
private:
    HWND m_hwnd;
    HDC m_hdc;
    int m_internalWidth;
    int m_internalHeight;
    int m_screenWidth;
    int m_screenHeight;
    bool m_fullscreen;
    bool m_paused;
    bool m_initialized;
    
    std::unique_ptr<LowResFramebuffer> m_framebuffer;
    std::chrono::high_resolution_clock::time_point m_startTime;
    float m_time;
    
    void initializeGraphics();
    void cleanup();
    
    static Engine* s_instance;
    
    void applyGraphicsSettings();
    void updateShaderUniforms(); 
};

#endif