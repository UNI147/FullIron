#include <iostream>
#include <windows.h>
#include "Shader.h"
#include "Tetrahedron.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad.h"

#pragma comment(lib, "opengl32.lib")

const int WIDTH = 800;
const int HEIGHT = 600;

// Объявляем глобальные переменные с префиксом g_
HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;

Shader* g_shader = nullptr;
Tetrahedron* g_tetrahedron = nullptr;
Texture* g_texture = nullptr;

float g_lastTime = 0.0f;

// Объявляем глобальные матрицы ДО их использования
glm::mat4 g_model = glm::mat4(1.0f);
glm::mat4 g_view = glm::mat4(1.0f);
glm::mat4 g_projection = glm::mat4(1.0f);

// Объявляем функции ДО их использования
void UpdateMatrices();
void InitOpenGL();
void Render(float deltaTime);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Определяем UpdateMatrices ДО функции Render
void UpdateMatrices() {
    if (!g_tetrahedron) return;
    
    // Модельная матрица (вращение)
    g_model = glm::mat4(1.0f);
    g_model = glm::rotate(g_model, glm::radians(g_tetrahedron->getRotationAngle()), 
                          glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Видовая матрица (камера)
    g_view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f), // позиция камеры
        glm::vec3(0.0f, 0.0f, 0.0f), // цель камеры
        glm::vec3(0.0f, 1.0f, 0.0f)  // вектор "вверх"
    );
    
    // Матрица проекции
    g_projection = glm::perspective(
        glm::radians(45.0f), // поле зрения
        (float)WIDTH / (float)HEIGHT, // соотношение сторон
        0.1f, 100.0f // ближняя и дальняя плоскости
    );
}

// Инициализация OpenGL
void InitOpenGL() {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        24,
        8,
        0,
        0, 0, 0, 0
    };
    
    int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
    SetPixelFormat(g_hDC, pixelFormat, &pfd);
    g_hRC = wglCreateContext(g_hDC);
    wglMakeCurrent(g_hDC, g_hRC);
    
    if (!gladLoadGL()) {
        if (!gladLoadGLLoader((GLADloadproc)wglGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return;
        }
    }
    
    // Проверяем версию OpenGL
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    // Настройки OpenGL
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    // Создаем шейдер, тетраэдр и текстуру
    try {
        g_shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        g_tetrahedron = new Tetrahedron();
        g_texture = new Texture("resources/metalplate.png");
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize: " << e.what() << std::endl;
    }
}

// Отрисовка сцены
void Render(float deltaTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Обновляем тетраэдр
    if (g_tetrahedron) {
        g_tetrahedron->update(deltaTime);
        UpdateMatrices();
    }
    
    // Используем шейдер
    if (g_shader && g_tetrahedron && g_texture) {
        g_shader->use();
        
        // Передаем матрицы в шейдер
        glm::mat4 mvp = g_projection * g_view * g_model;
        g_shader->setMat4("mvp", glm::value_ptr(mvp));
        
        g_texture->bind(0);
        g_shader->setInt("texture1", 0);
        g_tetrahedron->draw();
    }
    
    SwapBuffers(g_hDC);
}

// Простой обработчик сообщений
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            glViewport(0, 0, width, height);
            return 0;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    // Регистрация класса окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TetrahedronWindow";
    wc.style = CS_OWNDC;
    RegisterClass(&wc);
    
    // Создание окна
    g_hWnd = CreateWindow("TetrahedronWindow", "FullIron",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);
    
    g_hDC = GetDC(g_hWnd);
    InitOpenGL();
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Основной цикл
    MSG msg = {};
    g_lastTime = static_cast<float>(GetTickCount()) / 1000.0f;
    
    while(true) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            float currentTime = static_cast<float>(GetTickCount()) / 1000.0f;
            float deltaTime = currentTime - g_lastTime;
            g_lastTime = currentTime;
            
            Render(deltaTime);
            Sleep(16); // ~60 FPS
        }
    }
    
    // Очистка
    delete g_shader;
    delete g_tetrahedron;
    delete g_texture;
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    
    return 0;
}