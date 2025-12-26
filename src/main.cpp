#include <iostream>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <cmath>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

const int WIDTH = 800;
const int HEIGHT = 600;

// Объявляем глобальные переменные
HWND hWnd;      // Дескриптор окна
HDC hDC;        // Контекст устройства
HGLRC hRC;      // Контекст OpenGL

// Простая структура для 3D точки
struct Vector3 {
    float x, y, z;
    Vector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
};

// Вершины тетраэдра
Vector3 tetraVertices[4] = {
    Vector3(0.0f, 0.5f, 0.0f),    // Верх
    Vector3(-0.5f, -0.5f, -0.5f), // Основание 1
    Vector3(0.5f, -0.5f, -0.5f),  // Основание 2
    Vector3(0.0f, -0.5f, 0.5f)    // Основание 3
};

// Грани тетраэдра
int tetraFaces[4][3] = {
    {0, 1, 2},
    {0, 2, 3},
    {0, 3, 1},
    {1, 3, 2}
};

float rotationAngle = 0.0f;

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
    
    int pixelFormat = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pixelFormat, &pfd);
    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
    
    // Настройки OpenGL
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

// Отрисовка тетраэдра
void DrawTetrahedron() {
    glPushMatrix();
    glRotatef(rotationAngle, 0.5f, 1.0f, 0.0f);
    
    // Рисуем каждую грань разным цветом
    GLfloat colors[4][3] = {
        {1.0f, 0.0f, 0.0f}, // Красный
        {0.0f, 1.0f, 0.0f}, // Зеленый
        {0.0f, 0.0f, 1.0f}, // Синий
        {1.0f, 1.0f, 0.0f}  // Желтый
    };
    
    for (int i = 0; i < 4; i++) {
        glColor3fv(colors[i]);
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < 3; j++) {
            Vector3 v = tetraVertices[tetraFaces[i][j]];
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
    }
    
    glPopMatrix();
}

// Отрисовка сцены
void Render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)WIDTH/(GLfloat)HEIGHT, 0.1f, 100.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(2.0f, 2.0f, 2.0f,  // Позиция камеры
              0.0f, 0.0f, 0.0f,  // Точка взгляда
              0.0f, 1.0f, 0.0f); // Вектор "вверх"
    
    DrawTetrahedron();
    
    SwapBuffers(hDC);
}

// Обработчик сообщений Windows
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
            return 0;
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
    wc.lpszClassName = "FullIronWindow";
    wc.style = CS_OWNDC;
    RegisterClass(&wc);
    
    // Создание окна
    HWND hWndLocal = CreateWindow("FullIronWindow", "FullIron - 3D Tetrahedron",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                        WIDTH, HEIGHT, NULL, NULL, hInstance, NULL);
    hWnd = hWndLocal;
    
    hDC = GetDC(hWnd);
    InitOpenGL();
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // Основной цикл
    MSG msg = {};
    while(true) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            rotationAngle += 0.5f;
            if(rotationAngle > 360.0f) rotationAngle -= 360.0f;
            Render();
            Sleep(16); // ~60 FPS
        }
    }
    
    // Очистка
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    
    return 0;
}