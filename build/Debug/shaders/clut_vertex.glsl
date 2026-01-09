#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 FragPos;
out vec2 TexCoord;
out vec4 ClipSpacePos;
out float VertexDepth;
out vec2 ScreenPos;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool enableVertexJitter;
uniform bool useVertexSnapping;
uniform float vertexSnapThreshold;
uniform float vertexJitterAmount;
uniform float time;
uniform vec2 resolution;
uniform float subPixelShift;

// Простой генератор шума
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// ВЕРТЕКСНЫЙ ДЖИТТЕР (ОБНОВЛЕН)
vec3 applyVertexJitter(vec3 position, bool enableJitter, bool enableSnapping, 
                       float snapThreshold, float jitterAmount, float time) {  // <-- Добавили jitterAmount
    vec3 finalPos = position;
    
    if (enableJitter && jitterAmount > 0.001) {
        // Используем настраиваемую величину дрожания
        float actualJitterAmount = jitterAmount;
        
        // Основные искажения с разными частотами
        float jitterX = sin(position.x * 5.0 + time * 1.0) * actualJitterAmount;
        float jitterY = cos(position.y * 5.0 + time * 0.8) * actualJitterAmount;
        float jitterZ = sin(position.z * 5.0 + time * 1.2) * actualJitterAmount;
        
        // Добавляем немного хаотического шума
        float noiseX = random(position.xy + vec2(time * 0.5)) * 0.5 - 0.25;
        float noiseY = random(position.yz + vec2(time * 0.7)) * 0.5 - 0.25;
        float noiseZ = random(position.zx + vec2(time * 0.3)) * 0.5 - 0.25;
        
        finalPos.x += jitterX + noiseX * actualJitterAmount * 0.3;
        finalPos.y += jitterY + noiseY * actualJitterAmount * 0.3;
        finalPos.z += jitterZ + noiseZ * actualJitterAmount * 0.3;
    }
    
    // Вершинное квантование (снэппинг)
    if (useVertexSnapping && snapThreshold > 0.001) {
        // Используем настраиваемый порог
        float snap = snapThreshold;
        finalPos.x = floor(finalPos.x / snap) * snap;
        finalPos.y = floor(finalPos.y / snap) * snap;
        finalPos.z = floor(finalPos.z / snap) * snap;
    }
    
    return finalPos;
}

void main() {
    // Применяем вершинный джиттер с настраиваемыми параметрами
    vec3 finalPos = applyVertexJitter(aPos, enableVertexJitter, useVertexSnapping, 
                                     vertexSnapThreshold, vertexJitterAmount, time);  // <-- передаем jitterAmount
    
    // Субпиксельное смещение
    if (subPixelShift > 0.0) {
        finalPos.x += sin(time * 0.5) * subPixelShift * 0.001;
        finalPos.z += cos(time * 0.7) * subPixelShift * 0.001;
    }
    
    // Вычисляем позиции
    vec4 worldPos = model * vec4(finalPos, 1.0);
    FragPos = worldPos.xyz;
    TexCoord = aTexCoord;
    
    // Вычисляем позицию в пространстве отсечения
    ClipSpacePos = mvp * vec4(finalPos, 1.0);
    
    // РАСЧЕТ ЭКРАННЫХ КООРДИНАТ для аффинного текстурирования
    vec3 ndc = ClipSpacePos.xyz / ClipSpacePos.w;
    ScreenPos = (ndc.xy + 1.0) * 0.5 * resolution;
    
    // Сохраняем глубину
    VertexDepth = ClipSpacePos.z / ClipSpacePos.w;
    
    gl_Position = ClipSpacePos;
}