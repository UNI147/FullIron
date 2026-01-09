#ifndef CLUTTEXTURE_H
#define CLUTTEXTURE_H

#include "glad.h"
#include <vector>
#include <string>
#include <memory>

class CLUTTexture {
public:
    static const int PALETTE_SIZE = 256;
    
    // Конструктор с автоматической генерацией палитры
    CLUTTexture(const std::string& imagePath, 
                bool generatePalette = true,
                const std::string& customPaletteName = "");
    
    // Конструктор с использованием существующей палитры
    CLUTTexture(const std::string& imagePath,
                const std::vector<unsigned char>& palette);
    
    ~CLUTTexture();
    
    void bind(GLuint unit = 0) const;
    GLuint getID() const { return textureID; }
    GLuint getPaletteTextureID() const { return paletteTextureID; }
    
    // Методы для работы с палитрой
    const std::vector<unsigned char>& getPalette() const { return palette; }
    void applyNewPalette(const std::vector<unsigned char>& newPalette);
    void regeneratePalette();
    
private:
    GLuint textureID;
    GLuint paletteTextureID;
    int width, height;
    std::string imagePath;
    std::vector<unsigned char> palette;
    std::vector<unsigned char> indexedData;
    
    void loadImage();
    void setupTextures();
    
    std::vector<unsigned char> applyPaletteWithDithering(
        const unsigned char* data, 
        const std::vector<unsigned char>& palette);
};

#endif