#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "CLUTTexture.h"
#include "PaletteManager.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <cfloat>
#include <fstream>
#include <sstream>
#include "GraphicsSettings.h"

// Конструктор с автоматической генерацией палитры
CLUTTexture::CLUTTexture(const std::string& imagePath, 
                        bool generatePalette,
                        const std::string& customPaletteName) 
    : width(0), height(0), textureID(0), paletteTextureID(0),
      imagePath(imagePath) {
    
    // Загружаем изображение
    loadImage();
    
    if (width == 0 || height == 0) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        // Не создаем текстуры, если изображение не загружено
        return;
    }
    
    // Только 256-цветные палитры
    const int paletteSize = PALETTE_SIZE;
    
    try {
        // Получаем или генерируем палитру
        PaletteManager& pm = PaletteManager::getInstance();
        std::string paletteName = customPaletteName.empty() ? 
                                 pm.generatePaletteName(imagePath) : 
                                 customPaletteName;
        
        if (generatePalette || !pm.hasPalette(paletteName)) {
            // Генерируем новую палитру
            std::cout << "Generating new 256-color palette for: " << imagePath << std::endl;
            palette = pm.generatePaletteForTexture(imagePath, paletteSize, paletteName);
        } else {
            // Используем существующую палитру
            std::cout << "Using existing palette: " << paletteName << std::endl;
            palette = pm.getPalette(paletteName);
            
            if (palette.empty()) {
                // Если палитра не найдена, генерируем новую
                palette = pm.generatePaletteForTexture(imagePath, paletteSize, paletteName);
            }
        }
        
        // Применяем палитру с дизерингом
        int channels = 4; // RGBA
        std::vector<unsigned char> rgbaData(width * height * channels);
        
        // Конвертируем данные в RGBA
        for (int i = 0; i < width * height; i++) {
            rgbaData[i * 4] = indexedData[i * 4];
            rgbaData[i * 4 + 1] = indexedData[i * 4 + 1];
            rgbaData[i * 4 + 2] = indexedData[i * 4 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
        
        indexedData = applyPaletteWithDithering(rgbaData.data(), palette);
        
        // Создаем текстуры
        setupTextures();
        
    } catch (const std::exception& e) {
        std::cerr << "Error in CLUTTexture constructor: " << e.what() << std::endl;
        // Очищаем ресурсы при ошибке
        if (textureID) glDeleteTextures(1, &textureID);
        if (paletteTextureID) glDeleteTextures(1, &paletteTextureID);
        textureID = 0;
        paletteTextureID = 0;
    }
}

// Конструктор с использованием существующей палитры
CLUTTexture::CLUTTexture(const std::string& imagePath,
                        const std::vector<unsigned char>& externalPalette)
    : width(0), height(0), textureID(0), paletteTextureID(0),
      imagePath(imagePath), palette(externalPalette) {
    
    // Проверяем размер палитры
    if (palette.size() / 4 != PALETTE_SIZE) {
        std::cerr << "Warning: Palette size mismatch. Expected " 
                  << PALETTE_SIZE << " colors, got " 
                  << palette.size() / 4 << std::endl;
        if (palette.size() / 4 < PALETTE_SIZE) {
            // Дополняем палитру черным цветом
            palette.resize(PALETTE_SIZE * 4, 0);
        }
    }
    
    // Загружаем изображение
    loadImage();
    
    if (width == 0 || height == 0) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        return;
    }
    
    // Применяем переданную палитру с дизерингом
    int channels = 4;
    std::vector<unsigned char> rgbaData(width * height * channels);
    
    for (int i = 0; i < width * height; i++) {
        rgbaData[i * 4] = indexedData[i * 4];
        rgbaData[i * 4 + 1] = indexedData[i * 4 + 1];
        rgbaData[i * 4 + 2] = indexedData[i * 4 + 2];
        rgbaData[i * 4 + 3] = 255;
    }
    
    indexedData = applyPaletteWithDithering(rgbaData.data(), palette);
    
    // Создаем текстуры
    setupTextures();
}

void CLUTTexture::loadImage() {
    stbi_set_flip_vertically_on_load(true);
    int channels;
    unsigned char* data = stbi_load(imagePath.c_str(), &width, &height, &channels, 4);
    
    if (!data) {
        std::cerr << "Failed to load image: " << imagePath << std::endl;
        
        // Создаем простую текстуру по умолчанию
        width = 64;
        height = 64;
        indexedData.resize(width * height * 4);
        
        // Шахматный паттерн
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                bool isDark = ((x / 8) + (y / 8)) % 2 == 0;
                
                indexedData[idx] = isDark ? 100 : 200;
                indexedData[idx + 1] = isDark ? 100 : 200;
                indexedData[idx + 2] = isDark ? 100 : 200;
                indexedData[idx + 3] = 255;
            }
        }
        return;
    }
    
    // Копируем данные
    indexedData.resize(width * height * 4);
    std::copy(data, data + width * height * 4, indexedData.begin());
    
    stbi_image_free(data);
}

void CLUTTexture::setupTextures() {
    // Проверяем наличие данных
    if (width == 0 || height == 0 || indexedData.empty()) {
        std::cerr << "Cannot setup textures: no image data" << std::endl;
        return;
    }
    
    // Создаем текстуру индексов
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // POINT SAMPLING - СТРОГО
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Подготавливаем данные индексов (8 бит)
    std::vector<unsigned char> indexData(width * height);
    
    for (int i = 0; i < width * height; i++) {
        int bestIndex = 0;
        float bestDistance = FLT_MAX;
        
        unsigned char r = indexedData[i * 4];
        unsigned char g = indexedData[i * 4 + 1];
        unsigned char b = indexedData[i * 4 + 2];
        
        for (int j = 0; j < PALETTE_SIZE; j++) {
            float dr = static_cast<float>(r) - static_cast<float>(palette[j * 4]);
            float dg = static_cast<float>(g) - static_cast<float>(palette[j * 4 + 1]);
            float db = static_cast<float>(b) - static_cast<float>(palette[j * 4 + 2]);
            float distance = dr * dr + dg * dg + db * db;
            
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIndex = j;
            }
        }
        
        indexData[i] = static_cast<unsigned char>(bestIndex);
    }
    
    // Пробуем использовать GL_R8 формат
    bool r8Supported = true;
    
    // Очищаем все предыдущие ошибки OpenGL
    while (glGetError() != GL_NO_ERROR);
    
    // Пробуем загрузить с GL_R8
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, 
                 GL_RED, GL_UNSIGNED_BYTE, indexData.data());
    
    // Проверяем наличие ошибок OpenGL
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "GL_R8 not supported (error: " << error 
                  << "), falling back to GL_RGBA format" << std::endl;
        r8Supported = false;
        
        // Пересоздаем текстуру
        glDeleteTextures(1, &textureID);
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        // Устанавливаем параметры фильтрации
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Конвертируем в RGBA формат
        std::vector<unsigned char> rgbaData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            unsigned char idx = indexData[i];
            rgbaData[i * 4] = idx;     // R = индекс
            rgbaData[i * 4 + 1] = 0;   // G = 0
            rgbaData[i * 4 + 2] = 0;   // B = 0
            rgbaData[i * 4 + 3] = 255; // A = 255
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Failed to create texture with GL_RGBA format: " << error << std::endl;
            glDeleteTextures(1, &textureID);
            textureID = 0;
            return;
        }
    } else {
        std::cout << "GL_R8 format supported for index texture" << std::endl;
    }

    // Создаем текстуру палитры
    glGenTextures(1, &paletteTextureID);
    glBindTexture(GL_TEXTURE_1D, paletteTextureID);
    
    // ТОЧЕЧНАЯ ФИЛЬТРАЦИЯ ДЛЯ ПАЛИТРЫ
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    int actualPaletteSize = static_cast<int>(palette.size() / 4);
    if (actualPaletteSize > 0) {
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, actualPaletteSize, 0, 
                     GL_RGBA, GL_UNSIGNED_BYTE, palette.data());
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Failed to create palette texture: " << error << std::endl;
            glDeleteTextures(1, &paletteTextureID);
            paletteTextureID = 0;
        }
    } else {
        std::cerr << "Cannot create palette texture: palette is empty" << std::endl;
        glDeleteTextures(1, &paletteTextureID);
        paletteTextureID = 0;
    }
}

std::vector<unsigned char> CLUTTexture::applyPaletteWithDithering(
    const unsigned char* data, const std::vector<unsigned char>& pal) {
    
    int pixelCount = width * height;
    std::vector<unsigned char> result(pixelCount * 4, 0);
    const int paletteSize = PALETTE_SIZE;
    
    // Используем настраиваемый размер матрицы
    int matrixSize = GraphicsSettings::DITHERING_MATRIX_SIZE;
    if (matrixSize < 2) matrixSize = 2;
    if (matrixSize > 16) matrixSize = 16;
    
    // Создаем матрицу Байера указанного размера
    std::vector<float> bayerMatrix(matrixSize * matrixSize);
    for (int y = 0; y < matrixSize; y++) {
        for (int x = 0; x < matrixSize; x++) {
            float value = 0.0f;
            int mask = matrixSize;
            
            // Генерация матрицы Байера
            for (int level = 1; level < matrixSize; level *= 2) {
                mask >>= 1;
                if ((y & level) != 0) value += 1.0f;
                if ((x & level) != 0) value += 2.0f;
            }
            
            bayerMatrix[y * matrixSize + x] = value / (matrixSize * matrixSize);
        }
    }
    
    // Используем настраиваемую силу дизеринга
    float ditherStrength = GraphicsSettings::DITHERING_STRENGTH;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            
            // Исходный цвет
            float r = static_cast<float>(data[idx * 4]);
            float g = static_cast<float>(data[idx * 4 + 1]);
            float b = static_cast<float>(data[idx * 4 + 2]);
            
            // Пороговый дизеринг с матрицей Байера
            float threshold = (bayerMatrix[(y % matrixSize) * matrixSize + (x % matrixSize)] - 0.5f) * ditherStrength;
            r = std::clamp(r + threshold, 0.0f, 255.0f);
            g = std::clamp(g + threshold, 0.0f, 255.0f);
            b = std::clamp(b + threshold, 0.0f, 255.0f);
            
            // Находим ближайший цвет в палитре
            int bestIndex = 0;
            float bestDistance = FLT_MAX;
            
            for (int j = 0; j < paletteSize; j++) {
                float dr = r - static_cast<float>(pal[j * 4]);
                float dg = g - static_cast<float>(pal[j * 4 + 1]);
                float db = b - static_cast<float>(pal[j * 4 + 2]);
                float distance = dr * dr + dg * dg + db * db;
                
                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestIndex = j;
                }
            }
            
            // Получаем выбранный цвет
            result[idx * 4] = pal[bestIndex * 4];
            result[idx * 4 + 1] = pal[bestIndex * 4 + 1];
            result[idx * 4 + 2] = pal[bestIndex * 4 + 2];
            result[idx * 4 + 3] = 255;
        }
    }
    
    return result;
}

void CLUTTexture::applyNewPalette(const std::vector<unsigned char>& newPalette) {
    if (newPalette.size() / 4 != PALETTE_SIZE) {
        std::cerr << "Error: New palette must have " << PALETTE_SIZE << " colors" << std::endl;
        return;
    }
    
    palette = newPalette;
    
    // Пересчитываем индексы с новой палитрой
    std::vector<unsigned char> indexData(width * height);
    
    for (int i = 0; i < width * height; i++) {
        int bestIndex = 0;
        float bestDistance = FLT_MAX;
        
        unsigned char r = indexedData[i * 4];
        unsigned char g = indexedData[i * 4 + 1];
        unsigned char b = indexedData[i * 4 + 2];
        
        for (int j = 0; j < PALETTE_SIZE; j++) {
            float dr = static_cast<float>(r) - static_cast<float>(palette[j * 4]);
            float dg = static_cast<float>(g) - static_cast<float>(palette[j * 4 + 1]);
            float db = static_cast<float>(b) - static_cast<float>(palette[j * 4 + 2]);
            float distance = dr * dr + dg * dg + db * db;
            
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIndex = j;
            }
        }
        
        indexData[i] = static_cast<unsigned char>(bestIndex);
    }
    
    // Обновляем текстуру индексов
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Простой подход: попробуем обновить как GL_R8, если ошибка - используем GL_RGBA
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, 
                    GL_RED, GL_UNSIGNED_BYTE, indexData.data());
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        // Если GL_R8 не поддерживается, используем GL_RGBA
        glGetError(); // Очищаем ошибку
        
        // Конвертируем в RGBA для запасного варианта
        std::vector<unsigned char> rgbaData(width * height * 4);
        for (int i = 0; i < width * height; i++) {
            rgbaData[i * 4] = indexData[i];
            rgbaData[i * 4 + 1] = 0;
            rgbaData[i * 4 + 2] = 0;
            rgbaData[i * 4 + 3] = 255;
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, 
                        GL_RGBA, GL_UNSIGNED_BYTE, rgbaData.data());
    }
    
    // Обновляем текстуру палитры
    glBindTexture(GL_TEXTURE_1D, paletteTextureID);
    int actualPaletteSize = static_cast<int>(palette.size() / 4);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, actualPaletteSize, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, palette.data());
}

void CLUTTexture::regeneratePalette() {
    PaletteManager& pm = PaletteManager::getInstance();
    palette = pm.generatePaletteForTexture(imagePath, PALETTE_SIZE);
    applyNewPalette(palette);
}

CLUTTexture::~CLUTTexture() {
    if (textureID) glDeleteTextures(1, &textureID);
    if (paletteTextureID) glDeleteTextures(1, &paletteTextureID);
}

void CLUTTexture::bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glActiveTexture(GL_TEXTURE0 + unit + 1);
    glBindTexture(GL_TEXTURE_1D, paletteTextureID);
}