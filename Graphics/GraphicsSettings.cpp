#include "GraphicsSettings.h"

namespace GraphicsSettings {
    // Значения по умолчанию
    float VERTEX_JITTER_AMOUNT = 0.012f;
    float VERTEX_SNAP_THRESHOLD = 0.02f;
    float DITHERING_STRENGTH = 32.0f;
    int DITHERING_MATRIX_SIZE = 4;
    

    bool NEEDS_SHADER_UPDATE = false;
    
    void markForShaderUpdate() {
        NEEDS_SHADER_UPDATE = true;
    }
    
    void applySettingsToEngine() {
        if (NEEDS_SHADER_UPDATE) {
            std::cout << "Graphics settings updated, shaders need refresh" << std::endl;
            NEEDS_SHADER_UPDATE = false;
        }
    }
    
    // Обновляем loadSettings для установки флага:
    void loadSettings(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "Graphics settings file not found, using defaults" << std::endl;
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            // Пропускаем комментарии и пустые строки
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            std::istringstream iss(line);
            std::string key;
            if (std::getline(iss, key, '=')) {
                std::string value;
                if (std::getline(iss, value)) {
                    // Убираем пробелы
                    key.erase(0, key.find_first_not_of(" \t"));
                    key.erase(key.find_last_not_of(" \t") + 1);
                    value.erase(0, value.find_first_not_of(" \t"));
                    value.erase(value.find_last_not_of(" \t") + 1);
                    
                    try {
                        if (key == "VERTEX_JITTER_AMOUNT") {
                            VERTEX_JITTER_AMOUNT = std::stof(value);
                        } else if (key == "VERTEX_SNAP_THRESHOLD") {
                            VERTEX_SNAP_THRESHOLD = std::stof(value);
                        } else if (key == "DITHERING_STRENGTH") {
                            DITHERING_STRENGTH = std::stof(value);
                        } else if (key == "DITHERING_MATRIX_SIZE") {
                            DITHERING_MATRIX_SIZE = std::stoi(value);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing graphics setting '" << key << "': " << e.what() << std::endl;
                    }
                }
            }
            markForShaderUpdate();
        }
        
        std::cout << "Graphics settings loaded from " << filename << std::endl;
        file.close();
    }
    
    void saveSettings(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to save graphics settings to " << filename << std::endl;
            return;
        }
        
        file << "# Graphics Settings Configuration\n";
        file << "# Modify these values to change rendering behavior\n\n";
        
        file << "# Величина дрожания вершин (0.0 - 1.0)\n";
        file << "VERTEX_JITTER_AMOUNT = " << VERTEX_JITTER_AMOUNT << "\n\n";
        
        file << "# Порог снэппинга вершин (0.0 - 1.0)\n";
        file << "VERTEX_SNAP_THRESHOLD = " << VERTEX_SNAP_THRESHOLD << "\n\n";
        
        file << "# Сила дизеринга (0.0 - 100.0)\n";
        file << "DITHERING_STRENGTH = " << DITHERING_STRENGTH << "\n\n";
        
        file << "# Размер матрицы дизеринга (2, 4, 8, 16)\n";
        file << "DITHERING_MATRIX_SIZE = " << DITHERING_MATRIX_SIZE << "\n";
        
        file.close();
        std::cout << "Graphics settings saved to " << filename << std::endl;
    }
}