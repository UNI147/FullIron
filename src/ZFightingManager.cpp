#include "ZFightingManager.h"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ZFightingManager& ZFightingManager::getInstance() {
    static ZFightingManager instance;
    return instance;
}

void ZFightingManager::enablePrevention(bool enable) {
    preventionEnabled = enable;
    if (enable) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, zBias * 100000.0f);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void ZFightingManager::setZBias(float bias) {
    zBias = bias;
    
    if (preventionEnabled) {
        glPolygonOffset(1.0f, zBias * 100000.0f);
    }
}

void ZFightingManager::applyDepthBias(float baseBias) {
    if (!preventionEnabled) return;
    
    // Сохраняем оригинальные настройки
    glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &originalPolygonOffsetFactor);
    glGetFloatv(GL_POLYGON_OFFSET_UNITS, &originalPolygonOffsetUnits);
    
    // Применяем смещение
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, (zBias + baseBias) * 100000.0f);
}

bool ZFightingManager::supportsFragDepth() {
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    if (extensions) {
        std::string extensionsStr(reinterpret_cast<const char*>(extensions));
        return extensionsStr.find("GL_ARB_fragment_shader") != std::string::npos;
    }
    return true;
}

void ZFightingManager::resetDepthBias() {
    if (!preventionEnabled) return;
    
    glPolygonOffset(originalPolygonOffsetFactor, originalPolygonOffsetUnits);
}

void ZFightingManager::enablePaintersAlgorithm(bool enable) {
    usePaintersAlgorithm = enable;
    
    if (enable) {
        // Сохраняем оригинальные состояния
        glGetBooleanv(GL_DEPTH_TEST, &originalDepthTest);
        glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&originalDepthFunc));
        glGetBooleanv(GL_POLYGON_OFFSET_FILL, &originalPolygonOffset);
        
        // Отключаем Z-буфер полностью для Painter's algorithm
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POLYGON_OFFSET_FILL);
        
        // Включаем прозрачность для правильного наложения
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Отключаем Z-fighting prevention
        preventionEnabled = false;
        
    } else {
        // Восстанавливаем оригинальные состояния
        if (originalDepthTest) {
            glEnable(GL_DEPTH_TEST);
        }
        glDepthFunc(originalDepthFunc);
        
        if (originalPolygonOffset) {
            glEnable(GL_POLYGON_OFFSET_FILL);
        }
        
        glDisable(GL_BLEND);
        
        // Восстанавливаем Z-fighting prevention если нужно
        if (preventionEnabled) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0f, zBias * 100000.0f);
        }
    }
}

// Функция для сортировки полигонов внутри объекта
void ZFightingManager::sortObjectPolygons(std::vector<float>& depths, std::vector<unsigned int>& indices) {
    if (!usePaintersAlgorithm || depths.size() != indices.size()) return;
    
    // Создаем пары глубина-индекс
    std::vector<std::pair<float, unsigned int>> depthIndexPairs;
    for (size_t i = 0; i < depths.size(); ++i) {
        depthIndexPairs.emplace_back(depths[i], indices[i]);
    }
    
    // Сортируем по глубине (дальние сначала)
    std::sort(depthIndexPairs.begin(), depthIndexPairs.end(),
              [](const auto& a, const auto& b) {
                  return a.first > b.first;
              });
    
    // Обновляем индексы
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = depthIndexPairs[i].second;
    }
}

void ZFightingManager::sortByDepth(std::vector<glm::mat4>& modelMatrices, const glm::mat4& view) {
    if (!usePaintersAlgorithm || modelMatrices.empty()) return;
    
    struct DepthInfo {
        float depth;
        size_t index;
        glm::mat4 matrix;
        
        bool operator<(const DepthInfo& other) const {
            return depth > other.depth;
        }
    };
    
    std::vector<DepthInfo> depthInfos;
    depthInfos.reserve(modelMatrices.size());
    
    for (size_t i = 0; i < modelMatrices.size(); ++i) {
        glm::vec4 localCenter(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 worldCenter = modelMatrices[i] * localCenter;
        glm::vec4 viewCenter = view * worldCenter;
        
        float depth = viewCenter.z;
        depthInfos.push_back({depth, i, modelMatrices[i]});
    }
    
    std::sort(depthInfos.begin(), depthInfos.end());
    
    std::vector<glm::mat4> sortedMatrices;
    sortedMatrices.reserve(modelMatrices.size());
    
    for (const auto& info : depthInfos) {
        sortedMatrices.push_back(info.matrix);
    }
    
    modelMatrices = std::move(sortedMatrices);
}

void ZFightingManager::renderWithPaintersAlgorithm(const std::function<void()>& renderCallback) {
    if (!usePaintersAlgorithm) {
        renderCallback();
        return;
    }
    renderCallback();
}

void ZFightingManager::sortObjectPolygonsDetailed(std::vector<glm::vec3>& vertices, 
                                                 std::vector<unsigned int>& indices, 
                                                 const glm::mat4& modelView) {
    if (!usePaintersAlgorithm || indices.size() % 3 != 0) return;
    
    struct PolygonInfo {
        float avgDepth;
        unsigned int triangleIndex;
        glm::vec3 center;
        
        bool operator<(const PolygonInfo& other) const {
            return avgDepth > other.avgDepth; // Сортировка от дальних к ближним
        }
    };
    
    std::vector<PolygonInfo> polygonInfos;
    
    // Рассчитываем среднюю глубину для каждого треугольника
    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i]];
        glm::vec3 v1 = vertices[indices[i + 1]];
        glm::vec3 v2 = vertices[indices[i + 2]];
        
        // Преобразуем в пространство камеры
        glm::vec4 viewV0 = modelView * glm::vec4(v0, 1.0f);
        glm::vec4 viewV1 = modelView * glm::vec4(v1, 1.0f);
        glm::vec4 viewV2 = modelView * glm::vec4(v2, 1.0f);
        
        // Средняя глубина
        float avgDepth = (viewV0.z + viewV1.z + viewV2.z) / 3.0f;
        
        // Центр треугольника
        glm::vec3 center = (v0 + v1 + v2) / 3.0f;
        
        polygonInfos.push_back({avgDepth, static_cast<unsigned int>(i / 3), center});
    }
    
    // Сортируем по глубине
    std::sort(polygonInfos.begin(), polygonInfos.end());
    
    // Перестраиваем индексы
    std::vector<unsigned int> sortedIndices;
    sortedIndices.reserve(indices.size());
    
    for (const auto& info : polygonInfos) {
        size_t baseIdx = info.triangleIndex * 3;
        sortedIndices.push_back(indices[baseIdx]);
        sortedIndices.push_back(indices[baseIdx + 1]);
        sortedIndices.push_back(indices[baseIdx + 2]);
    }
    
    indices = std::move(sortedIndices);
}