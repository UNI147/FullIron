#include "ResourceManager.h"
#include <iostream>
#include <filesystem>

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::initialize() {
    std::cout << "Initializing Resource Manager..." << std::endl;
    // Создаем необходимые директории
    std::filesystem::create_directories("resources");
    std::filesystem::create_directories("shaders");
    std::filesystem::create_directories("scenes");
    
    std::cout << "Resource Manager initialized" << std::endl;
}

void ResourceManager::cleanup() {
    std::cout << "Cleaning up resources..." << std::endl;
    unloadAll();
    std::cout << "Resource Manager cleaned up" << std::endl;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, 
                                                   const std::string& vertexPath, 
                                                   const std::string& fragmentPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        std::cout << "Shader '" << name << "' already loaded" << std::endl;
        return it->second;
    }
    
    try {
        std::cout << "Loading shader: " << name << std::endl;
        auto shader = std::make_shared<Shader>(vertexPath.c_str(), fragmentPath.c_str());
        m_shaders[name] = shader;
        std::cout << "Shader loaded successfully: " << name << std::endl;
        return shader;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shader '" << name << "': " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    std::cerr << "Shader not found: " << name << std::endl;
    return nullptr;
}

std::shared_ptr<CLUTTexture> ResourceManager::loadTexture(const std::string& name, 
                                                         const std::string& path,
                                                         bool generatePalette) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        std::cout << "Texture '" << name << "' already loaded" << std::endl;
        return it->second;
    }
    
    try {
        std::cout << "Loading texture: " << name << " from " << path << std::endl;
        auto texture = std::make_shared<CLUTTexture>(path, generatePalette);
        m_textures[name] = texture;
        std::cout << "Texture loaded successfully: " << name 
                  << " (ID: " << texture->getID() << ")" << std::endl;
        return texture;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture '" << name << "': " << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<CLUTTexture> ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second;
    }
    std::cerr << "Texture not found: " << name << std::endl;
    return nullptr;
}

void ResourceManager::unloadAll() {
    std::cout << "Unloading " << m_shaders.size() << " shaders and " 
              << m_textures.size() << " textures" << std::endl;
    
    m_shaders.clear();
    m_textures.clear();
}

size_t ResourceManager::getResourceCount() const {
    return m_shaders.size() + m_textures.size();
}