#include "ZFightingManager.h"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ZFightingManager& ZFightingManager::getInstance() {
    static ZFightingManager instance;
    return instance;
}

void ZFightingManager::enablePaintersAlgorithm(bool enable) {
    usePaintersAlgorithm = enable;
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
        
    glDisable(GL_BLEND);
}

void ZFightingManager::sortByDepth(std::vector<glm::mat4>& modelMatrices, const glm::mat4& view) {
    if (!usePaintersAlgorithm || modelMatrices.empty()) return;
    
    struct DepthInfo {
        float depth;
        glm::mat4 matrix;
        size_t originalIndex;
        
        bool operator<(const DepthInfo& other) const {
            // Сортировка от дальних к ближним (от большего Z к меньшему)
            return depth > other.depth;
        }
    };
    
    std::vector<DepthInfo> depthInfos;
    depthInfos.reserve(modelMatrices.size());
    
    for (size_t i = 0; i < modelMatrices.size(); ++i) {
        // Берем позицию центра объекта (в локальных координатах это (0,0,0))
        glm::vec4 localPos(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 worldPos = modelMatrices[i] * localPos;
        glm::vec4 viewPos = view * worldPos;
        
        // Глубина в пространстве камеры (чем больше Z, тем дальше)
        float depth = viewPos.z;
        depthInfos.push_back({depth, modelMatrices[i], i});
    }
    
    // Сортируем по глубине (от дальних к ближним)
    std::sort(depthInfos.begin(), depthInfos.end());
    
    // Обновляем массив матриц в отсортированном порядке
    for (size_t i = 0; i < modelMatrices.size(); ++i) {
        modelMatrices[i] = depthInfos[i].matrix;
    }
}