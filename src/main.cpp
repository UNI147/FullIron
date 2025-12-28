#include <iostream>
#include <windows.h>
#include <fstream>
#include "Shader.h"
#include "Tetrahedron.h"
#include "LowResFramebuffer.h"
#include "CLUTTexture.h"
#include "PaletteManager.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glad.h"
#include "GraphicsConfig.h"

#pragma comment(linker, "/subsystem:console")
#pragma comment(lib, "opengl32.lib")

// Точное разрешение 320x240 (4:3)
const int INTERNAL_WIDTH = GraphicsConfig::INTERNAL_WIDTH;
const int INTERNAL_HEIGHT = GraphicsConfig::INTERNAL_HEIGHT;

// Разрешение окна (кратное 320x240 для честного масштабирования)
const int WINDOW_WIDTH = 960;  // 320 * 3
const int WINDOW_HEIGHT = 720; // 240 * 3

HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;
LowResFramebuffer* g_lowResFBO = nullptr;
Shader* g_clutShader = nullptr;
Tetrahedron* g_tetrahedron = nullptr;
CLUTTexture* g_clutTexture = nullptr;

float g_rotationAngle = 0.0f;

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
                glViewport(0, 0, width, height);
            }
            return 0;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                DestroyWindow(hWnd);
            }
            return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool InitOpenGL() {
    std::cout << "Initializing FullIron" << std::endl;

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
    
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    SetPixelFormat(g_hDC, pixelFormat, &pfd);
    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);
    
    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Internal resolution: " << INTERNAL_WIDTH << "x" << INTERNAL_HEIGHT << std::endl;
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // ЖЕСТКОЕ ПРИНУДИТЕЛЬНОЕ УСТАНОВЛЕНИЕ ТОЧЕЧНОЙ ФИЛЬТРАЦИИ
    std::cout << "FORCING POINT SAMPLING FOR ALL TEXTURES..." << std::endl;
    
    try {
        // Загружаем CLUT шейдер
        std::cout << "Loading CLUT shader..." << std::endl;
        g_clutShader = new Shader("shaders/clut_vertex.glsl", "shaders/clut_fragment.glsl");
        
        // Создаем тетраэдр
        std::cout << "Creating tetrahedron..." << std::endl;
        g_tetrahedron = new Tetrahedron();
        
        // Загружаем текстуру (строго с 256-цветной палитрой)
        std::cout << "Loading texture with 256-color palette..." << std::endl;
        std::ifstream testFile("resources/metalplate.png");
        if (!testFile.good()) {
            std::cerr << "Texture not found, using default..." << std::endl;
            g_clutTexture = new CLUTTexture("");
        } else {
            testFile.close();
            g_clutTexture = new CLUTTexture("resources/metalplate.png", true);
            std::cout << "256-color CLUT texture loaded" << std::endl;
        }
        
        // Создаем фреймбуфер СТРОГО 320x240
        std::cout << "Creating 320x240 framebuffer (RGB555 emulated)..." << std::endl;
        g_lowResFBO = new LowResFramebuffer(INTERNAL_WIDTH, INTERNAL_HEIGHT);
        
        std::cout << "========================================" << std::endl;
        std::cout << "FULLIRON" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

void Render() {
    // Просто рендерим без лишних проверок
    
    g_rotationAngle += 0.5f;
    if (g_rotationAngle > 360.0f) g_rotationAngle -= 360.0f;
    
    // МАТРИЦА МОДЕЛИ
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(g_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    
    // КАМЕРА
    glm::mat4 view = glm::lookAt(
        glm::vec3(2.5f, 2.2f, 2.5f),
        glm::vec3(0.0f, 0.3f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    // ПРОЕКЦИЯ 4:3
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(INTERNAL_WIDTH) / static_cast<float>(INTERNAL_HEIGHT),
        0.1f, 100.0f
    );
    
    // 1. РЕНДЕР ВО ФРЕЙМБУФЕР 320x240
    g_lowResFBO->beginRender();
    
    // ЦВЕТ ФОНА: темно-серый
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Используем CLUT шейдер
    g_clutShader->use();
    
    // Передаем матрицы
    glm::mat4 mvp = projection * view * model;
    g_clutShader->setMat4("mvp", glm::value_ptr(mvp));
    
    // Привязываем текстуры
    g_clutTexture->bind(0);
    g_clutShader->setInt("indexTexture", 0);
    g_clutShader->setInt("paletteTexture", 1);
    
    // Включаем дизеринг
    g_clutShader->setBool("useDithering", true);
    g_clutShader->setVec2("resolution", INTERNAL_WIDTH, INTERNAL_HEIGHT);
    
    // Рисуем тетраэдр
    g_tetrahedron->draw();
    
    g_lowResFBO->endRender();
    
    // 2. ВЫВОД НА ЭКРАН С RGB555 ЭМУЛЯЦИЕЙ
    g_lowResFBO->renderToScreen(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    SwapBuffers(g_hDC);
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "FullIronWindow";
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    
    RegisterClass(&wc);
    
    g_hWnd = CreateWindow(
        "FullIronWindow", 
        "FullIron",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    g_hDC = GetDC(g_hWnd);
    
    if (!InitOpenGL()) {
        std::cerr << "Critical initialization failed!" << std::endl;
        MessageBox(NULL, "Failed to initialize OpenGL!", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    
    MSG msg = {};
    bool running = true;
    
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        if (running) {
            Render();
        }
        
        Sleep(16);
    }
    
    delete g_clutShader;
    delete g_tetrahedron;
    delete g_clutTexture;
    delete g_lowResFBO;
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    
    std::cout << "Application terminated." << std::endl;
    return 0;
}