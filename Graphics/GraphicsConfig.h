#ifndef GRAPHICSCONFIG_H
#define GRAPHICSCONFIG_H

#include <iostream>
#include <algorithm>
#include "GraphicsSettings.h"

namespace GraphicsConfig {
    constexpr int INTERNAL_WIDTH = 320;
    constexpr int INTERNAL_HEIGHT = 240;
    constexpr float ASPECT_RATIO = 4.0f / 3.0f;
    
    // Цветовые форматы
    enum class ColorFormat {
        RGB555     // 5-5-5 (эмулируется в шейдере)
    };
    
    constexpr bool FORCE_POINT_SAMPLING = true;
    constexpr bool USE_INTEGER_SCALING = true;
    constexpr bool ENABLE_DITHERING = true;
    
    // НАСТРОЙКИ
    constexpr bool ENABLE_AFFINE_TEXTURING = true;
    constexpr bool ENABLE_VERTEX_JITTER = true;
    
    // Эти параметры теперь настраиваются
    constexpr bool USE_VERTEX_SNAPPING = ENABLE_VERTEX_JITTER;
    
    [[maybe_unused]] static bool isFormatSupported(ColorFormat format) {
        (void)format;
        return true;
    }
}

#endif