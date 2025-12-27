#include <iostream>
#include <windows.h>
#include <random>
#include "Shader.h"
#include "Tetrahedron.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glad.h"

#pragma comment(lib, "opengl32.lib")

// Изменено: Внутреннее разрешение 320x240
const int INTERNAL_WIDTH = 320;
const int INTERNAL_HEIGHT = 240;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// Добавлено: Буфер для внутреннего рендеринга
GLuint g_framebuffer = 0;
GLuint g_renderTexture = 0;
GLuint g_depthBuffer = 0;
Shader* g_postprocessShader = nullptr;
GLuint g_quadVAO = 0, g_quadVBO = 0;

// Объявляем глобальные переменные с префиксом g_
HWND g_hWnd = nullptr;
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;

Shader* g_shader = nullptr;
Shader* g_ditherShader = nullptr;
Tetrahedron* g_tetrahedron = nullptr;
Texture* g_texture = nullptr;
Texture* g_clutTexture = nullptr; // Для демонстрации CLUT-текстур

float g_lastTime = 0.0f;
bool g_useDithering = true;
bool g_useCLUT = false;

// Объявляем глобальные матрицы ДО их использования
glm::mat4 g_model = glm::mat4(1.0f);
glm::mat4 g_view = glm::mat4(1.0f);
glm::mat4 g_projection = glm::mat4(1.0f);

// Функции
void UpdateMatrices();
void InitOpenGL();
void Render(float deltaTime);
void RenderToFramebuffer(float deltaTime);
void RenderPostprocess();
void SetupFramebuffer();
void SetupPostprocessQuad();
void CreateTestCLUTTexture();
void ApplyDitheringToTexture();
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void UpdateMatrices() {
    if (!g_tetrahedron) return;
    
    g_model = glm::mat4(1.0f);
    g_model = glm::rotate(g_model, glm::radians(g_tetrahedron->getRotationAngle()), 
                          glm::vec3(0.0f, 1.0f, 0.0f));
    
    g_view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    g_projection = glm::perspective(
        glm::radians(45.0f),
        (float)INTERNAL_WIDTH / (float)INTERNAL_HEIGHT,
        0.1f, 100.0f
    );
}

void SetupFramebuffer() {
    // Создаем фреймбуфер
    glGenFramebuffers(1, &g_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
    
    // Создаем текстуру для цвета - RGB555 формат
    glGenTextures(1, &g_renderTexture);
    glBindTexture(GL_TEXTURE_2D, g_renderTexture);
    
    // Важно: используем RGB5 для 16-битного цвета (5-5-5-1, но альфа не используется)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5, INTERNAL_WIDTH, INTERNAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    
    // POINT SAMPLING - как в требованиях
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_renderTexture, 0);
    
    // Буфер глубины
    glGenRenderbuffers(1, &g_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, INTERNAL_WIDTH, INTERNAL_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_depthBuffer);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SetupPostprocessQuad() {
    float quadVertices[] = {
        // pos        // tex
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &g_quadVAO);
    glGenBuffers(1, &g_quadVBO);
    glBindVertexArray(g_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

// Создаем тестовую CLUT-текстуру
void CreateTestCLUTTexture() {
    // Создаем простую палитру (16 цветов для CLUT4, 256 для CLUT8)
    std::vector<unsigned char> clut;
    int paletteSize = 16; // Для CLUT4
    
    for (int i = 0; i < paletteSize; i++) {
        // Градиентная палитра
        float t = i / (float)(paletteSize - 1);
        clut.push_back(static_cast<unsigned char>(255 * t)); // R
        clut.push_back(static_cast<unsigned char>(255 * (1 - t))); // G
        clut.push_back(static_cast<unsigned char>(128 + 127 * sin(t * 3.14159))); // B
    }
    
    // Создаем индексированные данные (шахматный паттерн)
    std::vector<unsigned char> indexData(INTERNAL_WIDTH * INTERNAL_HEIGHT);
    for (int y = 0; y < INTERNAL_HEIGHT; y++) {
        for (int x = 0; x < INTERNAL_WIDTH; x++) {
            // Шахматный паттерн с градиентом
            int pattern = ((x / 16) + (y / 16)) % 2;
            int gradient = (x + y) % paletteSize;
            indexData[y * INTERNAL_WIDTH + x] = static_cast<unsigned char>(
                pattern == 0 ? gradient : (paletteSize - 1 - gradient)
            );
        }
    }
    
    // Создаем CLUT-текстуру
    g_clutTexture = new Texture(clut.data(), clut.size() * 3, 
                                indexData.data(), INTERNAL_WIDTH, INTERNAL_HEIGHT,
                                Texture::PixelFormat::CLUT4);
    
    std::cout << "Created test CLUT4 texture with " << paletteSize << " colors" << std::endl;
}

// Функция для применения дизеринга к текстуре (альтернативный метод)
void ApplyDitheringToTexture() {
    // Читаем данные из текстуры
    std::vector<unsigned char> pixels(INTERNAL_WIDTH * INTERNAL_HEIGHT * 3);
    glBindTexture(GL_TEXTURE_2D, g_renderTexture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    
    // Матрица Байера 4x4 для упорядоченного дизеринга
    static const float bayerMatrix4x4[4][4] = {
        { 0.0f/16.0f,  8.0f/16.0f,  2.0f/16.0f, 10.0f/16.0f },
        { 12.0f/16.0f,  4.0f/16.0f, 14.0f/16.0f,  6.0f/16.0f },
        { 3.0f/16.0f, 11.0f/16.0f,  1.0f/16.0f,  9.0f/16.0f },
        { 15.0f/16.0f,  7.0f/16.0f, 13.0f/16.0f,  5.0f/16.0f }
    };
    
    // Применяем дизеринг
    for (int y = 0; y < INTERNAL_HEIGHT; y++) {
        for (int x = 0; x < INTERNAL_WIDTH; x++) {
            int idx = (y * INTERNAL_WIDTH + x) * 3;
            
            // Порог из матрицы Байера
            float threshold = bayerMatrix4x4[x % 4][y % 4];
            
            // Применяем к каждому каналу
            for (int c = 0; c < 3; c++) {
                float value = pixels[idx + c] / 255.0f;
                
                // Добавляем шум дизеринга
                value += (threshold - 0.5f) * 0.1f; // 10% интенсивность дизеринга
                value = std::max(0.0f, std::min(1.0f, value));
                
                // Квантуем до 5 бит (32 уровня)
                int quantized = static_cast<int>(value * 31.0f + 0.5f);
                pixels[idx + c] = static_cast<unsigned char>((quantized * 255) / 31);
            }
        }
    }
    
    // Загружаем обратно в текстуру
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT, 
                    GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
}

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
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    
    try {
        // Шейдеры
        g_shader = new Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
        g_ditherShader = new Shader("shaders/vertex.glsl", "shaders/dither_fragment.glsl");
        g_postprocessShader = new Shader("shaders/postprocess_vertex.glsl", "shaders/postprocess_fragment.glsl");
        
        // Объекты
        g_tetrahedron = new Tetrahedron();
        
        // Текстуры - пробуем разные форматы
        g_texture = new Texture("resources/metalplate.png", Texture::PixelFormat::RGB555);
        
        // Создаем тестовую CLUT-текстуру
        CreateTestCLUTTexture();
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize: " << e.what() << std::endl;
    }
    
    SetupFramebuffer();
    SetupPostprocessQuad();
    
    std::cout << "\n=== Режимы рендеринга ===" << std::endl;
    std::cout << "Внутреннее разрешение: " << INTERNAL_WIDTH << "x" << INTERNAL_HEIGHT << std::endl;
    std::cout << "Формат цвета: RGB555 (16-bit)" << std::endl;
    std::cout << "Фильтрация текстур: POINT SAMPLING (отключена)" << std::endl;
    std::cout << "Дизеринг: " << (g_useDithering ? "ВКЛ" : "ВЫКЛ") << std::endl;
    std::cout << "CLUT-текстуры: " << (g_clutTexture ? "ПОДДЕРЖИВАЕТСЯ" : "НЕТ") << std::endl;
    std::cout << "========================\n" << std::endl;
}

void RenderToFramebuffer(float deltaTime) {
    glBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);
    glViewport(0, 0, INTERNAL_WIDTH, INTERNAL_HEIGHT);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (g_tetrahedron) {
        g_tetrahedron->update(deltaTime);
        UpdateMatrices();
    }
    
    if (g_shader && g_tetrahedron) {
        // Выбираем шейдер в зависимости от настроек
        Shader* currentShader = g_useDithering ? g_ditherShader : g_shader;
        currentShader->use();
        
        // Передаем матрицы
        glm::mat4 mvp = g_projection * g_view * g_model;
        currentShader->setMat4("mvp", glm::value_ptr(mvp));
        
        // Выбираем текстуру
        if (g_useCLUT && g_clutTexture) {
            g_clutTexture->bind(0);
        } else {
            g_texture->bind(0);
        }
        currentShader->setInt("texture1", 0);
        
        // Также передаем разрешение для дизеринга
        if (g_useDithering) {
            currentShader->setInt("screenWidth", INTERNAL_WIDTH);
            currentShader->setInt("screenHeight", INTERNAL_HEIGHT);
        }
        
        g_tetrahedron->draw();
    }
    
    // Альтернативный метод дизеринга (постобработка)
    if (g_useDithering) {
        // ApplyDitheringToTexture(); // Раскомментировать для CPU дизеринга
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPostprocess() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (g_postprocessShader) {
        g_postprocessShader->use();
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_renderTexture);
        g_postprocessShader->setInt("screenTexture", 0);
        
        glBindVertexArray(g_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void Render(float deltaTime) {
    RenderToFramebuffer(deltaTime);
    RenderPostprocess();
    
    // Обновляем заголовок окна с информацией
    static float titleTimer = 0.0f;
    titleTimer += deltaTime;
    if (titleTimer > 1.0f) {
        titleTimer = 0.0f;
        std::string title = "FullIron";
        title += g_useDithering ? "Dithering ON" : "Dithering OFF";
        title += " - ";
        title += g_useCLUT ? "CLUT Texture" : "RGB Texture";
        SetWindowText(g_hWnd, title.c_str());
    }
    
    SwapBuffers(g_hDC);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
            
        case WM_KEYDOWN:
            switch(wParam) {
                case VK_F1:
                    g_useDithering = !g_useDithering;
                    std::cout << "Dithering: " << (g_useDithering ? "ON" : "OFF") << std::endl;
                    break;
                case VK_F2:
                    g_useCLUT = !g_useCLUT;
                    std::cout << "CLUT texture: " << (g_useCLUT ? "ON" : "OFF") << std::endl;
                    break;
                case VK_ESCAPE:
                    PostQuitMessage(0);
                    break;
            }
            return 0;
            
        case WM_SIZE: {
            // Не изменяем внутреннее разрешение
            return 0;
        }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TetrahedronWindow";
    wc.style = CS_OWNDC;
    RegisterClass(&wc);
    
    g_hWnd = CreateWindow("TetrahedronWindow", "FullIron",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
    
    g_hDC = GetDC(g_hWnd);
    InitOpenGL();
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
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
            Sleep(16);
        }
    }
    
    delete g_shader;
    delete g_ditherShader;
    delete g_postprocessShader;
    delete g_tetrahedron;
    delete g_texture;
    delete g_clutTexture;
    
    glDeleteFramebuffers(1, &g_framebuffer);
    glDeleteTextures(1, &g_renderTexture);
    glDeleteRenderbuffers(1, &g_depthBuffer);
    glDeleteVertexArrays(1, &g_quadVAO);
    glDeleteBuffers(1, &g_quadVBO);
    
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(g_hRC);
    ReleaseDC(g_hWnd, g_hDC);
    
    return 0;
}