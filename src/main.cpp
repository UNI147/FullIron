#include <iostream>
#include <windows.h>
#include <fstream>
#include <algorithm>
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
#include "ZFightingManager.h"

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

ZFightingManager& g_zFightingManager = ZFightingManager::getInstance();
float g_time = 0.0f;

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
    
    // Настраиваем функции
    g_zFightingManager.enablePrevention(true);
    g_zFightingManager.setZBias(0.0001f);
    
    // ЖЕСТКОЕ ПРИНУДИТЕЛЬНОЕ УСТАНОВЛЕНИЕ ТОЧЕЧНОЙ ФИЛЬТРАЦИИ
    std::cout << "FORCING POINT SAMPLING FOR ALL TEXTURES..." << std::endl;
    
    try {
        // Загружаем CLUT шейдер
        std::cout << "Loading CLUT shader..." << std::endl;
        g_clutShader = new Shader("shaders/clut_vertex.glsl", "shaders/clut_fragment.glsl");
        
        // Создаем тетраэдр
        std::cout << "Creating tetrahedron..." << std::endl;
        g_tetrahedron = new Tetrahedron();
        std::cout << "Tetrahedron created successfully" << std::endl;
        
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
        std::cout << "CLUT Texture loaded, ID: " << g_clutTexture->getID() 
                << ", Palette ID: " << g_clutTexture->getPaletteTextureID() << std::endl;
        
        // Создаем фреймбуфер
        g_lowResFBO = new LowResFramebuffer(INTERNAL_WIDTH, INTERNAL_HEIGHT);
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

void Render() {
    g_time += 0.016f;
    g_rotationAngle = g_time * 30.0f;
    
    // МАТРИЦА МОДЕЛИ
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(g_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    
    // КАМЕРА
    glm::mat4 view = glm::lookAt(
        glm::vec3(2.0f, 1.5f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::vec3 cameraPos = glm::vec3(2.0f, 1.5f, 2.0f);
    
    // ПРОЕКЦИЯ 4:3
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(INTERNAL_WIDTH) / static_cast<float>(INTERNAL_HEIGHT),
        0.1f, 100.0f
    );
    
    // 1. РЕНДЕР ВО ФРЕЙМБУФЕР 320x240
    g_lowResFBO->beginRender();
    
    // Очистка
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // ВКЛЮЧАЕМ PAINTER'S ALGORITHM если нужно
    if (GraphicsConfig::USE_PAINTERS_ALGORITHM) {
        g_zFightingManager.enablePaintersAlgorithm(true);
        
        // Отключаем Z-буфер и включаем смешивание
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
    
    // НАСТРАИВАЕМ SHADER
    g_clutShader->use();
    
    // Передаем матрицы
    glm::mat4 mvp = projection * view * model;
    g_clutShader->setMat4GLM("mvp", &mvp[0][0]);
    g_clutShader->setMat4GLM("model", &model[0][0]);
    g_clutShader->setMat4GLM("projection", &projection[0][0]);
    
    g_clutShader->setBool("enableVertexJitter", GraphicsConfig::ENABLE_VERTEX_JITTER);
    g_clutShader->setBool("useVertexSnapping", GraphicsConfig::ENABLE_VERTEX_JITTER);
    g_clutShader->setFloat("vertexSnapThreshold", 0.01f);
    g_clutShader->setBool("useAffineTexturing", GraphicsConfig::ENABLE_AFFINE_TEXTURING);
    g_clutShader->setBool("enableZFightingPrevention", 
                         GraphicsConfig::ENABLE_Z_FIGHTING_PREVENTION && !GraphicsConfig::USE_PAINTERS_ALGORITHM);
    g_clutShader->setFloat("zBias", GraphicsConfig::Z_BIAS);
    g_clutShader->setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    g_clutShader->setFloat("time", g_time);
    g_clutShader->setVec2("resolution", (float)INTERNAL_WIDTH, (float)INTERNAL_HEIGHT);
    g_clutShader->setBool("useDithering", true);
    g_clutShader->setBool("usePaintersAlgorithm", GraphicsConfig::USE_PAINTERS_ALGORITHM);
    g_clutShader->setFloat("depthOffset", 0.0f);
    
    // Привязываем текстуры
    g_clutTexture->bind(0);
    g_clutShader->setInt("indexTexture", 0);
    g_clutShader->setInt("paletteTexture", 1);
    
    // УПРАВЛЕНИЕ РЕНДЕРИНГОМ
    if (GraphicsConfig::USE_PAINTERS_ALGORITHM) {
        std::vector<glm::mat4> objectModels;
        
        // Основная пирамидка - центральная
        glm::mat4 modelMain = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        modelMain = glm::rotate(modelMain, glm::radians(g_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        modelMain = glm::scale(modelMain, glm::vec3(1.0f));
        objectModels.push_back(modelMain);
        
        // Вторая пирамидка - справа и ближе к камере
        glm::mat4 modelRight = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.0f, 0.5f));
        modelRight = glm::rotate(modelRight, glm::radians(g_rotationAngle * 0.7f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelRight = glm::scale(modelRight, glm::vec3(0.8f));
        objectModels.push_back(modelRight);
        
        // Третья пирамидка - слева и дальше от камеры
        glm::mat4 modelLeft = glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 0.3f, -0.8f));
        modelLeft = glm::rotate(modelLeft, glm::radians(g_rotationAngle * 1.3f), glm::vec3(0.0f, 1.0f, 0.0f));
        modelLeft = glm::scale(modelLeft, glm::vec3(0.6f));
        objectModels.push_back(modelLeft);
        
        // СОРТИРОВКА ПО ГЛУБИНЕ (дальние объекты рендерятся первыми)
        g_zFightingManager.sortByDepth(objectModels, view);
        
        // Рендерим отсортированные объекты (от дальних к ближним)
        for (size_t i = 0; i < objectModels.size(); ++i) {
            glm::mat4 objMvp = projection * view * objectModels[i];
            g_clutShader->setMat4GLM("mvp", &objMvp[0][0]);
            g_clutShader->setMat4GLM("model", &objectModels[i][0][0]);
            
            // Устанавливаем смещение глубины для каждого объекта
            float depthOffset = static_cast<float>(i) * 0.001f;
            g_clutShader->setFloat("depthOffset", depthOffset);
            
            g_tetrahedron->draw();
        }
        
        // Восстанавливаем состояния OpenGL
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        g_zFightingManager.enablePaintersAlgorithm(false);
        
    }
    
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