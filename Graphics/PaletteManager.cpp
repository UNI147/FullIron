#include "PaletteManager.h"
#include "stb_image.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <queue>
#include <map>
#include <set>

PaletteManager& PaletteManager::getInstance() {
    static PaletteManager instance;
    return instance;
}

std::vector<unsigned char> PaletteManager::generatePaletteForTexture(
    const std::string& texturePath, 
    int paletteSize,
    const std::string& paletteName) {
    
    // Убедимся что используем 256 цветов
    if (paletteSize != 256) {
        std::cout << "Warning: Forcing palette size to 256 colors" << std::endl;
        paletteSize = 256;
    }
    
    std::cout << "Generating 256-color palette for texture: " << texturePath << std::endl;
    
    // Загружаем текстуру
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texturePath.c_str(), 
                                   &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture for palette generation: " 
                  << texturePath << std::endl;
        
        // Возвращаем стандартную палитру
        std::vector<unsigned char> defaultPalette(paletteSize * 4, 0);
        for (int i = 0; i < paletteSize; i++) {
            float intensity = (i * 255.0f) / (paletteSize - 1);
            defaultPalette[i * 4] = static_cast<unsigned char>(intensity);
            defaultPalette[i * 4 + 1] = static_cast<unsigned char>(intensity);
            defaultPalette[i * 4 + 2] = static_cast<unsigned char>(intensity);
            defaultPalette[i * 4 + 3] = 255;
        }
        return defaultPalette;
    }
    
    // Используем median cut для генерации палитры
    std::vector<unsigned char> palette = medianCut(data, width, height, 
                                                  channels, paletteSize);
    
    // Если текстура имеет альфа-канал, сохраняем его
    if (channels == 4) {
        // Для альфа-канала используем простую стратегию
        for (int i = 0; i < paletteSize; i++) {
            palette[i * 4 + 3] = 255; // Полная непрозрачность
        }
    }
    
    // Сохраняем палитру
    std::string name = paletteName.empty() ? 
                      generatePaletteName(texturePath) : paletteName;
    palettes[name] = palette;
    
    // Создаем текстуру палитры
    paletteTextures[name] = createPaletteTexture(palette);
    
    stbi_image_free(data);
    
    std::cout << "Palette generated successfully: " << name 
              << " (" << paletteSize << " colors)" << std::endl;
    
    return palette;
}

// Реализация Median Cut алгоритма
std::vector<unsigned char> PaletteManager::medianCut(
    const unsigned char* imageData, 
    int width, int height, 
    int channels, int paletteSize) {
    
    struct ColorBox {
        int minR = 0, maxR = 255;
        int minG = 0, maxG = 255;
        int minB = 0, maxB = 255;
        std::vector<unsigned int> colors;
        
        ColorBox() = default;
        
        int getLongestAxis() const {
            int rangeR = maxR - minR;
            int rangeG = maxG - minG;
            int rangeB = maxB - minB;
            
            if (rangeR >= rangeG && rangeR >= rangeB) return 0; // R
            if (rangeG >= rangeR && rangeG >= rangeB) return 1; // G
            return 2; // B
        }
        
        unsigned char getAverageColor(int channel) const {
            if (colors.empty()) return 0;
            
            long long sum = 0;
            for (unsigned int color : colors) {
                switch(channel) {
                    case 0: sum += (color >> 16) & 0xFF; break; // R
                    case 1: sum += (color >> 8) & 0xFF; break;  // G
                    case 2: sum += color & 0xFF; break;         // B
                }
            }
            return static_cast<unsigned char>(sum / static_cast<long long>(colors.size()));
        }
        
        // Для сравнения в priority_queue
        bool operator<(const ColorBox& other) const {
            return colors.size() < other.colors.size();
        }
    };
    
    // Собираем все уникальные цвета
    std::vector<unsigned int> allColors;
    std::set<unsigned int> uniqueColors;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * channels;
            unsigned char r = imageData[idx];
            unsigned char g = imageData[idx + 1];
            unsigned char b = imageData[idx + 2];
            
            unsigned int colorKey = (static_cast<unsigned int>(r) << 16) | 
                                   (static_cast<unsigned int>(g) << 8) | 
                                   static_cast<unsigned int>(b);
            
            if (uniqueColors.insert(colorKey).second) {
                allColors.push_back(colorKey);
            }
        }
    }
    
    // Если цветов меньше или равно нужному количеству, возвращаем их
    if (static_cast<int>(allColors.size()) <= paletteSize) {
        std::vector<unsigned char> palette(paletteSize * 4, 0);
        for (size_t i = 0; i < allColors.size(); i++) {
            unsigned int color = allColors[i];
            palette[i * 4] = static_cast<unsigned char>((color >> 16) & 0xFF);
            palette[i * 4 + 1] = static_cast<unsigned char>((color >> 8) & 0xFF);
            palette[i * 4 + 2] = static_cast<unsigned char>(color & 0xFF);
            palette[i * 4 + 3] = 255;
        }
        return palette;
    }
    
    // Начальный бокс со всеми цветами
    ColorBox initialBox;
    initialBox.colors = allColors;
    
    // Обновляем границы начального бокса
    initialBox.minR = 255; initialBox.maxR = 0;
    initialBox.minG = 255; initialBox.maxG = 0;
    initialBox.minB = 255; initialBox.maxB = 0;
    
    for (unsigned int color : allColors) {
        unsigned char r = static_cast<unsigned char>((color >> 16) & 0xFF);
        unsigned char g = static_cast<unsigned char>((color >> 8) & 0xFF);
        unsigned char bVal = static_cast<unsigned char>(color & 0xFF);
        
        initialBox.minR = std::min(initialBox.minR, static_cast<int>(r));
        initialBox.maxR = std::max(initialBox.maxR, static_cast<int>(r));
        initialBox.minG = std::min(initialBox.minG, static_cast<int>(g));
        initialBox.maxG = std::max(initialBox.maxG, static_cast<int>(g));
        initialBox.minB = std::min(initialBox.minB, static_cast<int>(bVal));
        initialBox.maxB = std::max(initialBox.maxB, static_cast<int>(bVal));
    }
    
    std::priority_queue<ColorBox> boxes;
    boxes.push(initialBox);
    
    // Разделяем боксы пока не получим нужное количество
    while (static_cast<int>(boxes.size()) < paletteSize && !boxes.empty()) {
        ColorBox box = boxes.top();
        boxes.pop();
        
        if (box.colors.size() <= 1) {
            boxes.push(box);
            continue;
        }
        
        // Находим самую длинную ось
        int axis = box.getLongestAxis();
        
        // Сортируем цвета по выбранной оси
        std::sort(box.colors.begin(), box.colors.end(), 
            [axis](unsigned int a, unsigned int b) {
                unsigned char aVal = 0, bVal = 0;
                switch(axis) {
                    case 0: // R
                        aVal = static_cast<unsigned char>((a >> 16) & 0xFF);
                        bVal = static_cast<unsigned char>((b >> 16) & 0xFF);
                        break;
                    case 1: // G
                        aVal = static_cast<unsigned char>((a >> 8) & 0xFF);
                        bVal = static_cast<unsigned char>((b >> 8) & 0xFF);
                        break;
                    case 2: // B
                        aVal = static_cast<unsigned char>(a & 0xFF);
                        bVal = static_cast<unsigned char>(b & 0xFF);
                        break;
                }
                return aVal < bVal;
            });
        
        // Находим медиану
        size_t medianIdx = box.colors.size() / 2;
        
        // Создаем два новых бокса
        ColorBox box1, box2;
        box1.colors.assign(box.colors.begin(), 
                          box.colors.begin() + medianIdx);
        box2.colors.assign(box.colors.begin() + medianIdx, 
                          box.colors.end());
        
        // Обновляем границы боксов
        auto updateBoxBounds = [](ColorBox& b) {
            if (b.colors.empty()) return;
            
            b.minR = 255; b.maxR = 0;
            b.minG = 255; b.maxG = 0;
            b.minB = 255; b.maxB = 0;
            
            for (unsigned int color : b.colors) {
                unsigned char r = static_cast<unsigned char>((color >> 16) & 0xFF);
                unsigned char g = static_cast<unsigned char>((color >> 8) & 0xFF);
                unsigned char bVal = static_cast<unsigned char>(color & 0xFF);
                
                b.minR = std::min(b.minR, static_cast<int>(r));
                b.maxR = std::max(b.maxR, static_cast<int>(r));
                b.minG = std::min(b.minG, static_cast<int>(g));
                b.maxG = std::max(b.maxG, static_cast<int>(g));
                b.minB = std::min(b.minB, static_cast<int>(bVal));
                b.maxB = std::max(b.maxB, static_cast<int>(bVal));
            }
        };
        
        updateBoxBounds(box1);
        updateBoxBounds(box2);
        
        if (!box1.colors.empty()) boxes.push(box1);
        if (!box2.colors.empty()) boxes.push(box2);
    }
    
    // Собираем средние цвета из всех боксов
    std::vector<unsigned char> palette(paletteSize * 4, 0);
    int idx = 0;
    
    while (!boxes.empty() && idx < paletteSize) {
        ColorBox box = boxes.top();
        boxes.pop();
        
        if (!box.colors.empty()) {
            palette[idx * 4] = box.getAverageColor(0);     // R
            palette[idx * 4 + 1] = box.getAverageColor(1); // G
            palette[idx * 4 + 2] = box.getAverageColor(2); // B
            palette[idx * 4 + 3] = 255;                    // A
            idx++;
        }
    }
    
    // Заполняем оставшиеся слоты черным цветом
    for (; idx < paletteSize; idx++) {
        palette[idx * 4] = 0;
        palette[idx * 4 + 1] = 0;
        palette[idx * 4 + 2] = 0;
        palette[idx * 4 + 3] = 255;
    }
    
    return palette;
}

bool PaletteManager::savePalette(const std::string& paletteName, 
                                const std::string& filename) {
    auto it = palettes.find(paletteName);
    if (it == palettes.end()) {
        std::cerr << "Palette not found: " << paletteName << std::endl;
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    const std::vector<unsigned char>& palette = it->second;
    int paletteSize = static_cast<int>(palette.size() / 4);
    
    // Записываем заголовок
    const char header[] = "PAL";
    file.write(header, 3);
    file.write(reinterpret_cast<const char*>(&paletteSize), sizeof(int));
    
    // Записываем данные палитры
    file.write(reinterpret_cast<const char*>(palette.data()), 
               static_cast<std::streamsize>(palette.size()));
    
    file.close();
    std::cout << "Palette saved to: " << filename << std::endl;
    
    return true;
}

std::vector<unsigned char> PaletteManager::loadPalette(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open palette file: " << filename << std::endl;
        return {};
    }
    
    char header[4];
    file.read(header, 3);
    header[3] = '\0';
    
    if (std::string(header) != "PAL") {
        std::cerr << "Invalid palette file format: " << filename << std::endl;
        return {};
    }
    
    int paletteSize;
    file.read(reinterpret_cast<char*>(&paletteSize), sizeof(int));
    
    std::vector<unsigned char> palette(paletteSize * 4);
    file.read(reinterpret_cast<char*>(palette.data()), 
              static_cast<std::streamsize>(palette.size()));
    
    file.close();
    
    // Сохраняем в кэше
    std::string paletteName = filename;
    palettes[paletteName] = palette;
    paletteTextures[paletteName] = createPaletteTexture(palette);
    
    std::cout << "Palette loaded from: " << filename 
              << " (" << paletteSize << " colors)" << std::endl;
    
    return palette;
}

std::vector<unsigned char> PaletteManager::getPalette(const std::string& paletteName) {
    auto it = palettes.find(paletteName);
    if (it != palettes.end()) {
        return it->second;
    }
    return {};
}

bool PaletteManager::hasPalette(const std::string& paletteName) {
    return palettes.find(paletteName) != palettes.end();
}

GLuint PaletteManager::createPaletteTexture(const std::vector<unsigned char>& palette) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_1D, textureID);
    
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    int paletteSize = static_cast<int>(palette.size() / 4);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, paletteSize, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, palette.data());
    
    return textureID;
}

std::string PaletteManager::generatePaletteName(const std::string& texturePath) {
    // Извлекаем имя файла без расширения
    size_t lastSlash = texturePath.find_last_of("/\\");
    size_t lastDot = texturePath.find_last_of(".");
    
    std::string filename = texturePath.substr(
        lastSlash != std::string::npos ? lastSlash + 1 : 0,
        lastDot != std::string::npos ? lastDot - (lastSlash + 1) : std::string::npos
    );
    
    return filename + "_palette";
}