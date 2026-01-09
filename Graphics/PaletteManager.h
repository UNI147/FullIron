#ifndef PALETTEMANAGER_H
#define PALETTEMANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "glad.h"

class PaletteManager {
public:
    static PaletteManager& getInstance();
    
    // Генерация палитры для текстуры
    std::vector<unsigned char> generatePaletteForTexture(
        const std::string& texturePath, 
        int paletteSize = 256,
        const std::string& paletteName = "");
    
    // Сохранение палитры в файл
    bool savePalette(const std::string& paletteName, 
                    const std::string& filename);
    
    // Загрузка палитры из файла
    std::vector<unsigned char> loadPalette(const std::string& filename);
    
    // Получение палитры по имени
    std::vector<unsigned char> getPalette(const std::string& paletteName);
    
    // Проверка существования палитры
    bool hasPalette(const std::string& paletteName);
    
    // Создание текстуры палитры
    GLuint createPaletteTexture(const std::vector<unsigned char>& palette);
    
    // Генерация имени палитры на основе имени текстуры
    std::string generatePaletteName(const std::string& texturePath);
    
private:
    PaletteManager() = default;
    ~PaletteManager() = default;
    
    // Алгоритм генерации палитр
    std::vector<unsigned char> medianCut(const unsigned char* imageData, 
                                        int width, int height, 
                                        int channels, int paletteSize);
    
    // Хранение палитр
    std::unordered_map<std::string, std::vector<unsigned char>> palettes;
    std::unordered_map<std::string, GLuint> paletteTextures;
};

#endif