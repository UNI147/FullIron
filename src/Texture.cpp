#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include <iostream>
#include <fstream>

Texture::Texture(const std::string& path, PixelFormat format) 
    : ID(0), width(0), height(0), nrChannels(0), pixelFormat(format), isIndexed(false) {
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    
    // Настройка параметров текстуры - POINT SAMPLING (как в требованиях)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Проверяем, не CLUT ли это текстура по расширению
    std::string ext = path.substr(path.find_last_of(".") + 1);
    if (ext == "clut" || ext == "pal") {
        // Загрузка CLUT файла
        std::ifstream file(path, std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            clut.resize(size);
            file.read(reinterpret_cast<char*>(clut.data()), size);
            file.close();
            
            std::cout << "Loaded CLUT palette: " << path << " (" << size << " bytes)" << std::endl;
            isIndexed = true;
        }
    } else {
        // Обычная текстура
        loadTexture(path);
    }
}

// Конструктор для создания индексированных текстур
Texture::Texture(const unsigned char* clutData, int clutSize, 
                 const unsigned char* indexData, int width, int height, PixelFormat format)
    : ID(0), width(width), height(height), nrChannels(3), 
      pixelFormat(format), isIndexed(true) {
    
    // Сохраняем CLUT
    if (clutData && clutSize > 0) {
        clut.assign(clutData, clutData + clutSize);
    }
    
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    
    // Настройка параметров текстуры - POINT SAMPLING
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Создаем индексированную текстуру
    createIndexedTexture(indexData, width, height);
}

Texture::~Texture() {
    glDeleteTextures(1, &ID);
}

void Texture::loadTexture(const std::string& path) {
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        uploadToGPU();
        
        // Применяем RGB555 квантование если нужно
        if (pixelFormat == PixelFormat::RGB555) {
            convertToRGB555(data, width * height * nrChannels);
        }
        
        std::cout << "Texture loaded: " << path << " (" << width << "x" << height 
                  << ", channels: " << nrChannels << ", format: ";
        
        switch(pixelFormat) {
            case PixelFormat::RGB555: std::cout << "RGB555"; break;
            case PixelFormat::CLUT8: std::cout << "CLUT8"; break;
            case PixelFormat::CLUT4: std::cout << "CLUT4"; break;
            default: std::cout << "RGBA8888"; break;
        }
        std::cout << ")" << std::endl;
        
        stbi_image_free(data);
    } else {
        std::cerr << "Failed to load texture: " << path << std::endl;
        // Создаем простую текстуру на случай ошибки
        unsigned char defaultTexture[] = {128, 128, 128, 255};
        width = height = 1;
        nrChannels = 3;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, defaultTexture);
    }
}

void Texture::createIndexedTexture(const unsigned char* indexData, int width, int height) {
    if (!indexData || width <= 0 || height <= 0) {
        std::cerr << "Invalid indexed texture data" << std::endl;
        return;
    }
    
    // Конвертируем индексированные данные в RGB
    std::vector<unsigned char> rgbData(width * height * 3);
    int paletteSize = clut.size() / 3; // Предполагаем RGB палитру
    
    for (int i = 0; i < width * height; i++) {
        unsigned char index = indexData[i];
        
        if (pixelFormat == PixelFormat::CLUT4) {
            // Для 4-битных текстур, байт содержит 2 индекса
            if (i % 2 == 0) {
                index = (indexData[i / 2] >> 4) & 0x0F;
            } else {
                index = indexData[i / 2] & 0x0F;
            }
        }
        
        // Проверяем границы палитры
        if (index * 3 + 2 < clut.size()) {
            rgbData[i * 3] = clut[index * 3];
            rgbData[i * 3 + 1] = clut[index * 3 + 1];
            rgbData[i * 3 + 2] = clut[index * 3 + 2];
        } else {
            // Запасной цвет если индекс вне палитры
            rgbData[i * 3] = 255;
            rgbData[i * 3 + 1] = 0;
            rgbData[i * 3 + 2] = 255; // Пурпурный
        }
    }
    
    // Загружаем в GPU
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData.data());
    
    std::cout << "Created indexed texture: " << width << "x" << height 
              << " (format: " << (pixelFormat == PixelFormat::CLUT4 ? "CLUT4" : "CLUT8") 
              << ", palette size: " << paletteSize << ")" << std::endl;
}

void Texture::uploadToGPU() {
    GLenum format;
    GLenum internalFormat = GL_RGB;
    
    if (nrChannels == 1) {
        format = GL_RED;
        internalFormat = GL_RED;
    } else if (nrChannels == 3) {
        format = GL_RGB;
        internalFormat = (pixelFormat == PixelFormat::RGB555) ? GL_RGB5 : GL_RGB;
    } else if (nrChannels == 4) {
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    } else {
        format = GL_RGB;
        internalFormat = GL_RGB;
    }
    
    // Сначала загружаем с нормальным форматом
    unsigned char* data = stbi_load("resources/metalplate.png", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
}

void Texture::convertToRGB555(unsigned char* data, int size) {
    // Конвертируем 24-bit RGB в 16-bit RGB555
    for (int i = 0; i < size; i += nrChannels) {
        unsigned char r = data[i];
        unsigned char g = (nrChannels > 1) ? data[i + 1] : r;
        unsigned char b = (nrChannels > 2) ? data[i + 2] : r;
        
        // Конвертируем в RGB555 (5 бит на канал)
        data[i] = (r >> 3) & 0x1F;
        data[i + 1] = (g >> 3) & 0x1F;
        data[i + 2] = (b >> 3) & 0x1F;
    }
}

void Texture::setCLUT(const unsigned char* clutData, int clutSize) {
    if (clutData && clutSize > 0) {
        clut.assign(clutData, clutData + clutSize);
        isIndexed = true;
        
        // Перезагружаем текстуру с новой палитрой
        glBindTexture(GL_TEXTURE_2D, ID);
        // Здесь нужно было бы переконвертировать индексированные данные
    }
}

void Texture::bind(GLuint unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}