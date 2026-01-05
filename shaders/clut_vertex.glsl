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
uniform float time;
uniform vec2 resolution;

// Генератор псевдослучайного шума с анимацией
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Анимированный джиттер
float animatedJitter(vec3 position, float time) {
    vec3 animatedPos = position + vec3(sin(time), cos(time * 0.7), sin(time * 1.3)) * 0.1;
    float noise = random(animatedPos.xy + vec2(time * 0.5));
    noise += random(animatedPos.yz + vec2(time * 0.3));
    noise += random(animatedPos.zx + vec2(time * 0.7));
    return (noise / 3.0) * 2.0 - 1.0; // Нормализация к [-1, 1]
}

// Функция вершинного джиттера
vec3 applyVertexJitter(vec3 position, bool enableJitter, bool enableSnapping, float snapThreshold, float time) {
    vec3 finalPos = position;
    
    // АНИМИРОВАННЫЙ ДЖИТТЕР
    if (enableJitter && !enableSnapping) {
        float jitterAmount = 0.005;
        float jitter = animatedJitter(position, time) * jitterAmount;
        
        finalPos += vec3(jitter);
    }
    
    return finalPos;
}

void main() {
    // Применяем вершинный джиттер
    vec3 finalPos = applyVertexJitter(aPos, enableVertexJitter, useVertexSnapping, vertexSnapThreshold, time);
    
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