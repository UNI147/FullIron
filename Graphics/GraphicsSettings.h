#ifndef GRAPHICSSETTINGS_H
#define GRAPHICSSETTINGS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace GraphicsSettings {
    // Конфигурируемые параметры с значениями по умолчанию
    extern float VERTEX_JITTER_AMOUNT;
    extern float VERTEX_SNAP_THRESHOLD;
    extern float DITHERING_STRENGTH;
    extern int DITHERING_MATRIX_SIZE;
    
    // Загрузка настроек из файла
    void loadSettings(const std::string& filename = "graphics_settings.ini");
    
    // Сохранение настроек в файл
    void saveSettings(const std::string& filename = "graphics_settings.ini");

    
    extern bool NEEDS_SHADER_UPDATE;
    
    void markForShaderUpdate();
    void applySettingsToEngine();
}

#endif