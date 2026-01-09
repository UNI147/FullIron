#include "LowResFramebuffer.h"
#include <iostream>
#include <algorithm>

LowResFramebuffer::LowResFramebuffer(int internalWidth, int internalHeight) 
    : internalWidth(internalWidth), internalHeight(internalHeight),
      screenShaderProgram(0), useIntegerScaling(true) {
    
    std::cout << "Creating Phase 1 framebuffer: " 
              << internalWidth << "x" << internalHeight << std::endl;
    
    // Создаем фреймбуфер
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, internalWidth, internalHeight, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    
    // СТРОГО ТОЧЕЧНАЯ ФИЛЬТРАЦИЯ (POINT SAMPLING)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
    
    // Создаем renderbuffer для глубины/трафарета
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, internalWidth, internalHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
    
    // Проверяем статус фреймбуфера
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Framebuffer not complete! Status: 0x" 
                  << std::hex << status << std::dec << std::endl;
        throw std::runtime_error("Failed to create framebuffer");
    }
    
    std::cout << "Framebuffer created successfully" << std::endl;
    std::cout << "RGB555 will be emulated in shader" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    setupQuad();
    setupScreenShader();
}

LowResFramebuffer::~LowResFramebuffer() {
    if (screenShaderProgram) {
        glDeleteProgram(screenShaderProgram);
    }
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
    }
    if (textureID) {
        glDeleteTextures(1, &textureID);
    }
    if (RBO) {
        glDeleteRenderbuffers(1, &RBO);
    }
    if (FBO) {
        glDeleteFramebuffers(1, &FBO);
    }
}

void LowResFramebuffer::setupScreenShader() {
    // Вершинный шейдер
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    // Фрагментный шейдер с ЖЕСТКОЙ RGB555 эмуляцией
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        
        uniform sampler2D screenTexture;
        uniform vec2 internalResolution;
        uniform vec2 screenResolution;
        uniform bool useDithering;
        uniform bool useIntegerScaling;
        
        // Матрица Байера 4x4 для дизеринга
        float bayer4x4[16] = float[](
            0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
            12.0/16.0, 4.0/16.0,  14.0/16.0, 6.0/16.0,
            3.0/16.0,  11.0/16.0, 1.0/16.0,  9.0/16.0,
            15.0/16.0, 7.0/16.0,  13.0/16.0, 5.0/16.0
        );
        
        // СТРОГАЯ КОНВЕРТАЦИЯ В RGB555 (5-5-5)
        vec3 toRGB555(vec3 color) {
            // Квантование до 5 бит на канал (32 уровня)
            vec3 quantized = floor(color * 31.0 + 0.5) / 31.0;
            return quantized;
        }
        
        // Дизеринг для RGB555
        vec3 applyRGB555Dithering(vec3 color, vec2 pixelPos) {
            // Пороговое значение из матрицы Байера
            int x = int(mod(pixelPos.x, 4.0));
            int y = int(mod(pixelPos.y, 4.0));
            float threshold = (bayer4x4[x + y * 4] - 0.5) * (1.0 / 31.0);
            
            // Применяем дизеринг
            vec3 dithered = color + vec3(threshold);
            dithered = clamp(dithered, 0.0, 1.0);
            
            // Квантуем
            return floor(dithered * 31.0 + 0.5) / 31.0;
        }
        
        // Честное масштабирование (integer scaling)
        vec2 getInternalUV(vec2 uv, vec2 internalRes, vec2 screenRes) {
            if (!useIntegerScaling) {
                return uv;
            }
            
            // Рассчитываем целочисленный масштаб
            vec2 scale = floor(screenRes / internalRes);
            vec2 maxScale = max(scale.x, scale.y) * internalRes;
            vec2 offset = (screenRes - maxScale) * 0.5;
            
            // Преобразуем координаты
            vec2 pixelPos = uv * screenRes;
            pixelPos -= offset;
            
            // Если пиксель вне области рендера
            if (pixelPos.x < 0.0 || pixelPos.x >= maxScale.x ||
                pixelPos.y < 0.0 || pixelPos.y >= maxScale.y) {
                return vec2(-1.0); // Маркер вне области
            }
            
            // Нормализуем к внутреннему разрешению
            return pixelPos / maxScale;
        }
        
        void main() {
            // Получаем координаты во внутреннем разрешении
            vec2 internalUV = getInternalUV(TexCoord, internalResolution, screenResolution);
            
            // Если вне области рендера - черный цвет
            if (internalUV.x < 0.0) {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                return;
            }
            
            // Читаем цвет из фреймбуфера (всегда RGBA8)
            vec3 color = texture(screenTexture, internalUV).rgb;
            
            // Получаем позицию пикселя во внутреннем разрешении для дизеринга
            vec2 pixelPos = internalUV * internalResolution;
            
            // ПРИМЕНЯЕМ RGB555 С ДИЗЕРИНГОМ
            if (useDithering) {
                color = applyRGB555Dithering(color, pixelPos);
            } else {
                color = toRGB555(color);
            }
            
            FragColor = vec4(color, 1.0);
        }
    )";
    
    // Компилируем шейдеры
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // Создаем программу
    screenShaderProgram = glCreateProgram();
    glAttachShader(screenShaderProgram, vertexShader);
    glAttachShader(screenShaderProgram, fragmentShader);
    glLinkProgram(screenShaderProgram);
    
    // Проверяем ошибки
    GLint success;
    char infoLog[512];
    glGetProgramiv(screenShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(screenShaderProgram, 512, NULL, infoLog);
        std::cerr << "Screen shader linking failed: " << infoLog << std::endl;
    }
    
    // Удаляем шейдеры
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void LowResFramebuffer::beginRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, internalWidth, internalHeight);
    
    // Очищаем с цветом, соответствующим RGB555
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LowResFramebuffer::endRender() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LowResFramebuffer::renderToScreen(int screenWidth, int screenHeight) {
    // Отключаем интерполяцию для честного масштабирования
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Очищаем черным для рамки
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Используем наш шейдер
    glUseProgram(screenShaderProgram);
    
    // Устанавливаем параметры
    GLint internalResLoc = glGetUniformLocation(screenShaderProgram, "internalResolution");
    if (internalResLoc != -1) {
        glUniform2f(internalResLoc, (float)internalWidth, (float)internalHeight);
    }
    
    GLint screenResLoc = glGetUniformLocation(screenShaderProgram, "screenResolution");
    if (screenResLoc != -1) {
        glUniform2f(screenResLoc, (float)screenWidth, (float)screenHeight);
    }
    
    GLint useDitheringLoc = glGetUniformLocation(screenShaderProgram, "useDithering");
    if (useDitheringLoc != -1) {
        glUniform1i(useDitheringLoc, 1); // Включаем дизеринг
    }
    
    GLint useIntegerScalingLoc = glGetUniformLocation(screenShaderProgram, "useIntegerScaling");
    if (useIntegerScalingLoc != -1) {
        glUniform1i(useIntegerScalingLoc, useIntegerScaling ? 1 : 0);
    }
    
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    
    // ВАЖНО: Используем GL_NEAREST для point sampling
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Устанавливаем uniform
    GLint loc = glGetUniformLocation(screenShaderProgram, "screenTexture");
    if (loc != -1) {
        glUniform1i(loc, 0);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
}

void LowResFramebuffer::setupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}