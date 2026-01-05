#ifndef GRAPHICSCONFIG_H
#define GRAPHICSCONFIG_H

#include <iostream>
#include <algorithm> // Добавляем для std::sort

namespace GraphicsConfig {
    constexpr int INTERNAL_WIDTH = 320;
    constexpr int INTERNAL_HEIGHT = 240;
    constexpr float ASPECT_RATIO = 4.0f / 3.0f;
    
    // Цветовые форматы
    enum class ColorFormat {
        RGB555     // 5-5-5 (эмулируется в шейдере)
    };
    
    constexpr bool FORCE_POINT_SAMPLING = true;  // GL_NEAREST всегда
    constexpr bool USE_INTEGER_SCALING = true;   // Честное масштабирование
    constexpr bool ENABLE_DITHERING = true;      // Дизеринг для 16-бит
    
    constexpr bool ENABLE_AFFINE_TEXTURING = true;
    constexpr bool ENABLE_VERTEX_JITTER = true;
    
    constexpr bool ENABLE_Z_FIGHTING_PREVENTION = false;
    constexpr float Z_BIAS = 0.0001f;
    constexpr bool USE_PAINTERS_ALGORITHM = true;        // Painter's algorithm
    
    // Демонстрационный режим
    constexpr bool DEMONSTRATE_Z_FIGHTING = false;
    
    [[maybe_unused]] static bool isFormatSupported(ColorFormat format) {
        (void)format;
        return true;
    }
}

#endif