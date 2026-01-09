#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

namespace ConfigParser {

struct GameObjectConfig {
    std::string name;
    std::string type;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 color = glm::vec3(1.0f);
    std::string texture = "";
    float rotationSpeed = 0.0f;
    bool isMainCamera = false;
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    std::string parent;
    std::vector<std::string> children;
};

struct SceneConfig {
    std::string name;
    glm::vec3 backgroundColor = glm::vec3(0.1f);
    std::vector<GameObjectConfig> objects;
};

bool parseSceneFile(const std::string& filePath, SceneConfig& config);
glm::vec3 parseVec3(const std::string& str);
float parseFloat(const std::string& str);

} // namespace ConfigParser

#endif