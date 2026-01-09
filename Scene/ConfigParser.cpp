#include "ConfigParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace ConfigParser {

bool parseSceneFile(const std::string& filePath, SceneConfig& config) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Cannot open scene file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    GameObjectConfig currentObject;
    bool inObject = false;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        
        // Удаляем комментарии
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Удаляем лишние пробелы в начале и конце
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        // Проверяем начало нового объекта
        if (line[0] == '[' && line.back() == ']') {
            if (inObject) {
                config.objects.push_back(currentObject);
            }
            
            currentObject = GameObjectConfig();
            currentObject.name = line.substr(1, line.size() - 2);
            inObject = true;
            continue;
        }
        
        if (!inObject) continue;
        
        // Разбираем пары ключ=значение
        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos) continue;
        
        std::string key = line.substr(0, equalsPos);
        std::string value = line.substr(equalsPos + 1);
        
        // Удаляем пробелы вокруг ключа
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        
        // Удаляем пробелы вокруг значения
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (key == "type") {
            currentObject.type = value;
        } else if (key == "position") {
            currentObject.position = parseVec3(value);
        } else if (key == "rotation") {
            currentObject.rotation = parseVec3(value);
        } else if (key == "scale") {
            currentObject.scale = parseVec3(value);
        } else if (key == "color") {
            currentObject.color = parseVec3(value);
        } else if (key == "texture") {
            currentObject.texture = value;
        } else if (key == "rotationSpeed") {
            currentObject.rotationSpeed = parseFloat(value);
        } else if (key == "isMainCamera") {
            currentObject.isMainCamera = (value == "true" || value == "1" || value == "yes");
        } else if (key == "fov") {
            currentObject.fov = parseFloat(value);
        } else if (key == "nearPlane") {
            currentObject.nearPlane = parseFloat(value);
        } else if (key == "farPlane") {
            currentObject.farPlane = parseFloat(value);
        } else if (key == "parent") {
            currentObject.parent = value;
        } else if (key == "children") {
            std::stringstream ss(value);
            std::string child;
            while (std::getline(ss, child, ',')) {
                // Удаляем пробелы у каждого имени ребенка
                child.erase(0, child.find_first_not_of(" \t"));
                child.erase(child.find_last_not_of(" \t") + 1);
                if (!child.empty()) {
                    currentObject.children.push_back(child);
                }
            }
        }
    }
    
    // Добавляем последний объект
    if (inObject) {
        config.objects.push_back(currentObject);
    }
    
    std::cout << "Parsed " << config.objects.size() << " objects from " << filePath << std::endl;
    return true;
}

glm::vec3 parseVec3(const std::string& str) {
    std::stringstream ss(str);
    std::string token;
    std::vector<float> values;
    
    while (std::getline(ss, token, ',')) {
        try {
            // Удаляем пробелы у токена
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            values.push_back(std::stof(token));
        } catch (...) {
            values.push_back(0.0f);
        }
    }
    
    if (values.size() >= 3) {
        return glm::vec3(values[0], values[1], values[2]);
    } else if (values.size() == 2) {
        return glm::vec3(values[0], values[1], 0.0f);
    } else if (values.size() == 1) {
        return glm::vec3(values[0], values[0], values[0]);
    }
    
    return glm::vec3(0.0f);
}

float parseFloat(const std::string& str) {
    try {
        return std::stof(str);
    } catch (...) {
        return 0.0f;
    }
}

} // namespace ConfigParser