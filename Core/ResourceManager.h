#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <memory>
#include <unordered_map>
#include <string>
#include "Graphics/Shader.h"
#include "Graphics/CLUTTexture.h"

class ResourceManager {
public:
    static ResourceManager& getInstance();
    
    void initialize();
    void cleanup();
    
    // Шейдеры
    std::shared_ptr<Shader> loadShader(const std::string& name, 
                                      const std::string& vertexPath, 
                                      const std::string& fragmentPath);
    std::shared_ptr<Shader> getShader(const std::string& name);
    
    // Текстуры
    std::shared_ptr<CLUTTexture> loadTexture(const std::string& name, 
                                            const std::string& path,
                                            bool generatePalette = true);
    std::shared_ptr<CLUTTexture> getTexture(const std::string& name);
    
    // Утилиты
    void unloadAll();
    size_t getResourceCount() const;
    
private:
    ResourceManager() = default;
    ~ResourceManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<CLUTTexture>> m_textures;
};

#endif