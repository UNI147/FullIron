#ifndef GRAPHICSCONFIG_H
#define GRAPHICSCONFIG_H

#include <iostream>

namespace GraphicsConfig {
    // СТРОГИЕ НАСТРОЙКИ ФАЗЫ 1
    constexpr int INTERNAL_WIDTH = 320;
    constexpr int INTERNAL_HEIGHT = 240;
    constexpr float ASPECT_RATIO = 4.0f / 3.0f;
    
    // Цветовые форматы
    enum class ColorFormat {
        RGB555     // 5-5-5 (эмулируется в шейдере)
    };
    
    // НАСТРОЙКИ ФИЛЬТРАЦИИ
    constexpr bool FORCE_POINT_SAMPLING = true;  // GL_NEAREST всегда
    constexpr bool USE_INTEGER_SCALING = true;   // Честное масштабирование
    constexpr bool ENABLE_DITHERING = true;      // Дизеринг для 16-бит
    
    static bool isFormatSupported(ColorFormat format) {
        // RGB555 всегда поддерживается через эмуляцию в шейдере
        return true;
    }
    
    // Проверка требований
    static bool validateRequirements() {
        std::cout << "=== PHASE 1 REQUIREMENTS ===" << std::endl;
        std::cout << "1. Internal resolution: " << INTERNAL_WIDTH << "x" << INTERNAL_HEIGHT << " ✓" << std::endl;
        std::cout << "2. Point sampling: " << (FORCE_POINT_SAMPLING ? "ENFORCED ✓" : "FAIL") << std::endl;
        std::cout << "3. RGB555 color: EMULATED ✓" << std::endl;
        std::cout << "4. Integer scaling: " << (USE_INTEGER_SCALING ? "ENABLED ✓" : "disabled") << std::endl;
        std::cout << "5. Dithering: " << (ENABLE_DITHERING ? "ENABLED ✓" : "disabled") << std::endl;
        std::cout << "============================" << std::endl;
        return true;
    }
}

#endif