#include "Engine.h"
#include "Graphics/Shader.h"
#include "Core/ResourceManager.h"
#include <iostream>
#include <windows.h>
#include <gl/GL.h>

Engine* Engine::s_instance = nullptr;

Engine::Engine(HWND hwnd, HDC hdc, int internalWidth, int internalHeight)
    : m_hwnd(hwnd), m_hdc(hdc), 
      m_internalWidth(internalWidth), m_internalHeight(internalHeight),
      m_screenWidth(0), m_screenHeight(0),
      m_fullscreen(false), m_paused(false),
      m_initialized(false), m_time(0.0f) {
    
    s_instance = this;
    
    std::cout << "Creating FullIron Engine" << std::endl;
    std::cout << "Internal resolution: " << m_internalWidth << "x" << m_internalHeight << std::endl;
    
    initializeGraphics();
    m_startTime = std::chrono::high_resolution_clock::now();
    m_initialized = true;
}

Engine::~Engine() {
    cleanup();
}

void Engine::initializeGraphics() {
    std::cout << "Initializing graphics system..." << std::endl;
    
    // ОСНОВНЫЕ НАСТРОЙКИ OPENGL
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    
    std::cout << "Using Painter's algorithm (strict order rendering)" << std::endl;
    
    try {
        // Создаем фреймбуфер низкого разрешения
        m_framebuffer = std::make_unique<LowResFramebuffer>(m_internalWidth, m_internalHeight);
        
        // Загружаем ресурсы
        ResourceManager& rm = ResourceManager::getInstance();
        rm.loadShader("clut", "shaders/clut_vertex.glsl", "shaders/clut_fragment.glsl");
        
        std::cout << "Graphics system initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize graphics: " << e.what() << std::endl;
        throw;
    }
}

void Engine::update(float deltaTime) {
    if (m_paused) return;
    
    m_time += deltaTime;
    
    // Обновление сцены
    SceneManager::getInstance().update(deltaTime);
}

void Engine::render() {
    if (!m_initialized) return;
    
    // 1. РЕНДЕР ВО ФРЕЙМБУФЕР 320x240
    m_framebuffer->beginRender();
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Настройка OpenGL для Painter's algorithm
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    
    // Рендер текущей сцены
    SceneManager::getInstance().render();
    
    m_framebuffer->endRender();
    
    // 2. ВЫВОД НА ЭКРАН С RGB555 ЭМУЛЯЦИЕЙ
    m_framebuffer->renderToScreen(m_screenWidth, m_screenHeight);
    
    SwapBuffers(m_hdc);
}

void Engine::setScreenSize(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
    std::cout << "Screen size set to: " << width << "x" << height << std::endl;
}

void Engine::setFullscreen(bool fullscreen) {
    if (fullscreen == m_fullscreen) return;
    
    m_fullscreen = fullscreen;
    std::cout << "Fullscreen mode: " << (fullscreen ? "ON" : "OFF") << std::endl;
}

void Engine::toggleFullscreen() {
    m_fullscreen = !m_fullscreen;
    std::cout << "Toggled fullscreen: " << (m_fullscreen ? "ON" : "OFF") << std::endl;
}

void Engine::setPaused(bool paused) {
    m_paused = paused;
    std::cout << "Engine " << (paused ? "paused" : "resumed") << std::endl;
}

void Engine::cleanup() {
    std::cout << "Cleaning up engine..." << std::endl;
    
    // Очистка систем
    SceneManager::getInstance().cleanup();
    ResourceManager::getInstance().cleanup();
    
    m_framebuffer.reset();
    m_initialized = false;
}