#ifndef ZFIGHTINGMANAGER_H
#define ZFIGHTINGMANAGER_H

#include "glad.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

class ZFightingManager {
public:
    static ZFightingManager& getInstance();
    
    void enablePrevention(bool enable);
    void setZBias(float bias);
    void applyDepthBias(float baseBias = 0.0001f);
    void resetDepthBias();
    
    // Метод для проверки поддержки gl_FragDepth
    bool supportsFragDepth();
    
    // Painter's algorithm функции
    void enablePaintersAlgorithm(bool enable);
    void sortByDepth(std::vector<glm::mat4>& modelMatrices, const glm::mat4& view);
    void sortObjectPolygons(std::vector<float>& depths, std::vector<unsigned int>& indices);
    void renderWithPaintersAlgorithm(const std::function<void()>& renderCallback);
    
    // Детальная сортировка полигонов внутри объекта
    void sortObjectPolygonsDetailed(std::vector<glm::vec3>& vertices, 
                                    std::vector<unsigned int>& indices, 
                                    const glm::mat4& modelView);
    
    // Геттеры для состояния
    bool isPreventionEnabled() const { return preventionEnabled; }
    bool isPaintersAlgorithmEnabled() const { return usePaintersAlgorithm; }
    float getZBias() const { return zBias; }
    
private:
    ZFightingManager() = default;
    ~ZFightingManager() = default;
    
    bool preventionEnabled = true;
    float zBias = 0.0001f;
    bool usePaintersAlgorithm = false;
    
    // Для сохранения оригинальных состояний OpenGL
    GLfloat originalPolygonOffsetFactor = 0.0f;
    GLfloat originalPolygonOffsetUnits = 0.0f;
    GLenum originalDepthFunc = GL_LESS;
    GLboolean originalDepthTest = GL_TRUE;
    GLboolean originalPolygonOffset = GL_FALSE;
};

#endif