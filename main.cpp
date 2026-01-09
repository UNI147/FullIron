#include <iostream>
#include <windows.h>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include "Core/Engine.h"
#include "Core/ResourceManager.h"
#include "Graphics/LowResFramebuffer.h"
#include "Graphics/CLUTTexture.h"
#include "Graphics/PaletteManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glad.h"
#include "Graphics/GraphicsConfig.h"
#include "Graphics/ZFightingManager.h"
#include "Scene/SceneManager.h"
#include "Scene/MainScene.h"

#pragma comment(linker, "/subsystem:console")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "gdi32.lib")

// Точное разрешение 320x240 (4:3)
const int INTERNAL_WIDTH = GraphicsConfig::INTERNAL_WIDTH;
const int INTERNAL_HEIGHT = GraphicsConfig::INTERNAL_HEIGHT;

// Полноэкранное разрешение (будет определяться автоматически)
int g_screenWidth = 0;
int g_screenHeight = 0;

HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
bool g_fullscreen = true;
Engine* g_engine = nullptr;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            if (width > 0 && height > 0) {
                g_screenWidth = width;
                g_screenHeight = height;
                if (g_engine) {
                    g_engine->setScreenSize(width, height);
                }
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hWnd);
            }
            // Переключение полноэкранного режима по F11
            if (wParam == VK_F11) {
                if (g_engine) {
                    g_engine->toggleFullscreen();
                }
            }
            return 0;
        case WM_ACTIVATE:
            if (g_engine && wParam == WA_INACTIVE) {
                // При потере фокуса приостанавливаем движок
                g_engine->setPaused(true);
            } else if (g_engine && (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)) {
                // При получении фокуса возобновляем
                g_engine->setPaused(false);
            }
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool InitOpenGL(HWND hWnd, HDC& hDC, HGLRC& hRC, bool fullscreen) {
    std::cout << "Initializing FullIron Engine" << std::endl;
    
    // Получаем разрешение экрана
    g_screenWidth = GetSystemMetrics(SM_CXSCREEN);
    g_screenHeight = GetSystemMetrics(SM_CYSCREEN);
    std::cout << "Screen resolution: " << g_screenWidth << "x" << g_screenHeight << std::endl;
    
    // Настройки пиксельного формата
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        24, // depth buffer
        8,  // stencil buffer
        0,  // auxiliary buffers
        0, 0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
    
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Internal resolution: " << INTERNAL_WIDTH << "x" << INTERNAL_HEIGHT << std::endl;
    
    // Создаем движок
    try {
        g_engine = new Engine(hWnd, hDC, INTERNAL_WIDTH, INTERNAL_HEIGHT);
        g_engine->setScreenSize(g_screenWidth, g_screenHeight);
        g_engine->setFullscreen(fullscreen);
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize engine: " << e.what() << std::endl;
        return false;
    }
}

void ToggleFullscreen(HWND hWnd, bool& fullscreen, int& width, int& height) {
    fullscreen = !fullscreen;
    
    if (fullscreen) {
        // Получаем текущие настройки дисплея
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (DWORD)GetSystemMetrics(SM_CXSCREEN);
        dmScreenSettings.dmPelsHeight = (DWORD)GetSystemMetrics(SM_CYSCREEN);
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        
        // Устанавливаем полноэкранный режим
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            MessageBox(NULL, "Could not switch to fullscreen mode!", "Error", MB_OK | MB_ICONERROR);
            fullscreen = false;
            return;
        }
        
        // Изменяем стиль окна
        SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hWnd, HWND_TOP, 0, 0, 
                     GetSystemMetrics(SM_CXSCREEN), 
                     GetSystemMetrics(SM_CYSCREEN),
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        ShowCursor(FALSE);
        
        // Обновляем размеры
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    } else {
        // Восстанавливаем стандартные настройки дисплея
        ChangeDisplaySettings(NULL, 0);
        
        // Возвращаем оконный режим
        SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
        
        // Устанавливаем размеры окна (например, 960x720)
        int windowWidth = 960;
        int windowHeight = 720;
        int windowX = (GetSystemMetrics(SM_CXSCREEN) - windowWidth) / 2;
        int windowY = (GetSystemMetrics(SM_CYSCREEN) - windowHeight) / 2;
        
        SetWindowPos(hWnd, HWND_TOP, windowX, windowY, 
                     windowWidth, windowHeight, 
                     SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        ShowCursor(TRUE);
        
        // Обновляем размеры
        width = windowWidth;
        height = windowHeight;
    }
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    // Регистрация класса окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "FullIronWindow";
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Получаем разрешение экрана для полноэкранного режима
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    // Создаем окно в полноэкранном режиме сразу
    DWORD windowStyle = WS_POPUP | WS_VISIBLE;
    int windowX = 0;
    int windowY = 0;
    
    // Если не в полноэкранном режиме, используем оконный стиль
    if (!g_fullscreen) {
        windowStyle = WS_OVERLAPPEDWINDOW;
        screenWidth = 960;
        screenHeight = 720;
        windowX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
        windowY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
    }
    
    g_hWnd = CreateWindow(
        "FullIronWindow", 
        "FullIron Engine",
        windowStyle,
        windowX, windowY,
        screenWidth, screenHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hWnd) {
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Сразу устанавливаем полноэкранный режим если нужно
    if (g_fullscreen) {
        // Устанавливаем дисплей в полноэкранный режим
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (DWORD)screenWidth;
        dmScreenSettings.dmPelsHeight = (DWORD)screenHeight;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            MessageBox(NULL, "Could not switch to fullscreen mode!", "Error", MB_OK | MB_ICONERROR);
            g_fullscreen = false;
        } else {
            // Обновляем окно для полноэкранного режима
            SetWindowPos(g_hWnd, HWND_TOP, 0, 0, screenWidth, screenHeight,
                        SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            ShowCursor(FALSE);
        }
    }
    
    g_hDC = GetDC(g_hWnd);
    
    if (!InitOpenGL(g_hWnd, g_hDC, g_hRC, g_fullscreen)) {
        std::cerr << "Critical initialization failed!" << std::endl;
        MessageBox(NULL, "Failed to initialize OpenGL!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    
    // Инициализация менеджера ресурсов
    ResourceManager::getInstance().initialize();
    
    // Регистрируем главную сцену
    auto mainScene = std::make_shared<MainScene>();
    SceneManager::getInstance().registerScene("main", mainScene);
    
    // Загружаем начальную сцену
    SceneManager::getInstance().loadScene("main");
    
    MSG msg = {};
    bool running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Обработка сообщений Windows
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (running && g_engine) {
            // Обновление и рендер движка
            g_engine->update(deltaTime);
            g_engine->render();
            
            // Ограничение FPS (приблизительно 60 FPS)
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
    
    // Очистка
    delete g_engine;
    
    // Выход из полноэкранного режима перед закрытием
    if (g_fullscreen) {
        ChangeDisplaySettings(NULL, 0);
        ShowCursor(TRUE);
    }
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    DestroyWindow(g_hWnd);
    
    std::cout << "Application terminated." << std::endl;
    return 0;
}