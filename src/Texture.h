#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad.h"
#include <string>
#include <vector>

class Texture {
public:
    enum class PixelFormat {
        RGBA8888,   // 32-bit
        RGB555,     // 16-bit
        CLUT8,      // 8-bit indexed
        CLUT4       // 4-bit indexed
    };
    
    Texture(const std::string& path, PixelFormat format = PixelFormat::RGBA8888);
    Texture(const unsigned char* clutData, int clutSize, const unsigned char* indexData, 
            int width, int height, PixelFormat format);
    ~Texture();
    
    void bind(GLuint unit = 0) const;
    GLuint getID() const { return ID; }
    PixelFormat getFormat() const { return pixelFormat; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // CLUT-функции
    void setCLUT(const unsigned char* clutData, int clutSize);
    const std::vector<unsigned char>& getCLUT() const { return clut; }
    
private:
    GLuint ID;
    int width, height, nrChannels;
    PixelFormat pixelFormat;
    std::vector<unsigned char> clut; // Color Look-Up Table
    bool isIndexed;
    
    void loadTexture(const std::string& path);
    void createIndexedTexture(const unsigned char* indexData, int width, int height);
    void uploadToGPU();
    void convertToRGB555(unsigned char* data, int size);
};
#endif