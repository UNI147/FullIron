#include <iostream>
#include <windows.h>
#include <fstream>
#include <algorithm>
#include "Shader.h"
#include "Cube.h"
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
Cube* g_cube1 = nullptr;
Cube* g_cube2 = nullptr;
CLUTTexture* g_clutTexture = nullptr;

float g_rotationAngle1 = 0.0f;
float g_rotationAngle2 = 0.0f;

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
    
    // ОСНОВНЫЕ НАСТРОЙКИ OPENGL
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    // Blending не нужен для непрозрачных объектов
    glDisable(GL_BLEND);
    
    std::cout << "Using Painter's algorithm (strict order rendering)" << std::endl;
    
    // УСТАНОВЛЕНИЕ ТОЧЕЧНОЙ ФИЛЬТРАЦИИ
    std::cout << "FORCING POINT SAMPLING FOR ALL TEXTURES..." << std::endl;
    
    try {
        // Загружаем CLUT шейдер
        std::cout << "Loading CLUT shader..." << std::endl;
        g_clutShader = new Shader("shaders/clut_vertex.glsl", "shaders/clut_fragment.glsl");
        
        // Создаем кубы
        std::cout << "Creating cubes..." << std::endl;
        g_cube1 = new Cube();
        g_cube2 = new Cube();
        std::cout << "Cubes created successfully" << std::endl;
        
        // Загружаем текстуру
        std::cout << "Loading texture with 256-color palette..." << std::endl;
        std::ifstream testFile("resources/transformer.png");
        if (!testFile.good()) {
            std::cerr << "Texture not found, using default..." << std::endl;
            g_clutTexture = new CLUTTexture("");
        } else {
            testFile.close();
            g_clutTexture = new CLUTTexture("resources/transformer.png", true);
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
    g_rotationAngle1 = g_time * 30.0f;  // Вращение вокруг Y для нижнего куба
    g_rotationAngle2 = g_time * 20.0f;  // Вращение вокруг X для верхнего куба
    
    // Приближаем камеру
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(INTERNAL_WIDTH) / static_cast<float>(INTERNAL_HEIGHT),
        0.1f, 100.0f
    );
    
    // 1. РЕНДЕР ВО ФРЕЙМБУФЕР 320x240 С PAINTER'S ALGORITHM
    g_lowResFBO->beginRender();
    
    // Очистка
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // НАСТРАИВАЕМ OPENGL ДЛЯ PAINTER'S ALGORITHM
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    
    glDisable(GL_BLEND);
    
    // Настраиваем шейдер
    g_clutShader->use();
    
    // Передаем матрицы и настройки
    g_clutShader->setMat4GLM("projection", &projection[0][0]);
    g_clutShader->setMat4GLM("view", &view[0][0]);
    
    g_clutShader->setBool("enableVertexJitter", GraphicsConfig::ENABLE_VERTEX_JITTER);
    g_clutShader->setBool("useVertexSnapping", GraphicsConfig::ENABLE_VERTEX_JITTER);
    g_clutShader->setFloat("vertexSnapThreshold", GraphicsConfig::VERTEX_SNAP_THRESHOLD);
    g_clutShader->setBool("useAffineTexturing", GraphicsConfig::ENABLE_AFFINE_TEXTURING);
    g_clutShader->setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
    g_clutShader->setFloat("time", g_time);
    g_clutShader->setVec2("resolution", (float)INTERNAL_WIDTH, (float)INTERNAL_HEIGHT);
    g_clutShader->setBool("useDithering", GraphicsConfig::ENABLE_DITHERING);
    g_clutShader->setFloat("subPixelShift", 0.08f);
    
    // Привязываем текстуры
    g_clutTexture->bind(0);
    g_clutShader->setInt("indexTexture", 0);
    g_clutShader->setInt("paletteTexture", 1);
    
    // СОЗДАЕМ И СОРТИРУЕМ ОБЪЕКТЫ ПО ГЛУБИНЕ
    
    // КУБ 1: левый нижний (БЛИЖЕ к камере)
    glm::mat4 model1 = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, -0.4f, 0.2f));
    model1 = glm::rotate(model1, glm::radians(g_rotationAngle1), glm::vec3(0.0f, 1.0f, 0.0f));
    model1 = glm::scale(model1, glm::vec3(0.5f));
    
    // КУБ 2: правый верхний (ДАЛЬШЕ от камеры)
    glm::mat4 model2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.4f, 0.2f, -0.3f));
    model2 = glm::rotate(model2, glm::radians(g_rotationAngle2), glm::vec3(1.0f, 0.0f, 0.0f));
    model2 = glm::scale(model2, glm::vec3(0.5f));
    
    // Создаем вектор для сортировки
    std::vector<std::pair<float, glm::mat4>> objects;
    
    // Рассчитываем глубину в пространстве камеры
    glm::vec4 pos1 = view * model1 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 pos2 = view * model2 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    objects.push_back({pos1.z, model1});
    objects.push_back({pos2.z, model2});
    
    // Сортируем по глубине (от дальних к ближним)
    std::sort(objects.begin(), objects.end(), 
        [](const std::pair<float, glm::mat4>& a, const std::pair<float, glm::mat4>& b) {
            return a.first < b.first;
        });
    
    // Рендерим отсортированные объекты (от дальних к ближним)
    for (const auto& obj : objects) {
        glm::mat4 mvp = projection * view * obj.second;
        g_clutShader->setMat4GLM("mvp", &mvp[0][0]);
        g_clutShader->setMat4GLM("model", &obj.second[0][0]);
        
        // Рисуем куб
        g_cube1->draw();
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
    delete g_cube1;
    delete g_cube2;
    delete g_clutTexture;
    delete g_lowResFBO;
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    
    std::cout << "Application terminated." << std::endl;
    return 0;
}