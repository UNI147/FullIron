#ifndef ZFIGHTINGMANAGER_H
#define ZFIGHTINGMANAGER_H

#include "glad.h"
#include <glm/glm.hpp>
#include <vector>

class ZFightingManager {
public:
    static ZFightingManager& getInstance();
    
    // Painter's algorithm функции
    void enablePaintersAlgorithm(bool enable);
    void sortByDepth(std::vector<glm::mat4>& modelMatrices, const glm::mat4& view);
    
    // Геттер
    bool isPaintersAlgorithmEnabled() const { return usePaintersAlgorithm; }
    
private:
    ZFightingManager() = default;
    ~ZFightingManager() = default;
    
    bool usePaintersAlgorithm = true;
};

#endif