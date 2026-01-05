#version 330 core
in vec3 FragPos;
in vec2 TexCoord;
in vec4 ClipSpacePos;
in float VertexDepth;
in vec2 ScreenPos;
out vec4 FragColor;

uniform sampler2D indexTexture;
uniform sampler1D paletteTexture;
uniform bool useAffineTexturing;
uniform bool useDithering;
uniform vec2 resolution;
uniform vec3 cameraPos;
uniform float time;
uniform bool enableZFightingPrevention;
uniform float zBias;
uniform bool usePaintersAlgorithm;
uniform float depthOffset;

// Матрица Байера для дизеринга
float bayer4x4[16] = float[](
    0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
    3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
);

// Проверка поддержки gl_FragDepth
#ifdef GL_ARB_fragment_shader
    #define SUPPORTS_FRAG_DEPTH 1
#else
    #define SUPPORTS_FRAG_DEPTH 0
#endif

// аффинное текстурирование - ЭМУЛЯЦИЯ СОФТВЕРНОГО РЕНДЕРЕРА
vec2 affineTextureCoord(vec2 texCoord, vec4 clipPos, vec2 screenPos) {
    if (!useAffineTexturing) {
        return texCoord;
    }
    
    // Используем экранные координаты для линейной интерполяции
    ivec2 pixelPos = ivec2(floor(screenPos));
    
    // Создаем аффинные искажения:
    // 1. Интерполяция без 1/w коррекции
    float w = clipPos.w;
    float affineFactor = 1.0 / max(w, 1.0);
    
    // 2. Эмуляция независимой интерполяции для каждой грани
    float gridX = float(pixelPos.x & 3) / 3.0; // Размер грани 4 пикселя
    float gridY = float(pixelPos.y & 3) / 3.0;
    
    vec2 affineDistortion = vec2(0.0);
    
    // 3. Добавляем характерные "разрывы" на гранях
    affineDistortion.x = sin(gridX * 10.0 + time * 2.0) * 0.01 * affineFactor;
    affineDistortion.y = cos(gridY * 10.0 + time * 2.0) * 0.01 * affineFactor;
    
    // 4. "Плывущий" эффект на удаленных полигонах
    float distanceEffect = 1.0 - min(affineFactor * 3.0, 1.0);
    affineDistortion.x += sin(texCoord.x * 30.0 + time) * 0.005 * distanceEffect;
    affineDistortion.y += cos(texCoord.y * 30.0 + time) * 0.005 * distanceEffect;
    
    // 5. Линейная интерполяция в экранном пространстве
    vec2 screenUV = screenPos / resolution;
    affineDistortion += (screenUV - texCoord) * 0.1 * distanceEffect;
    
    return texCoord + affineDistortion;
}

// Дизеринг
vec4 applyDithering(vec4 color, vec2 fragCoord) {
    if (!useDithering) return color;
    
    ivec2 pixelPos = ivec2(fragCoord);
    int bayerIndex = (pixelPos.x & 3) + ((pixelPos.y & 3) << 2);
    float threshold = bayer4x4[bayerIndex];
    
    // Дизеринг для ограниченной палитры
    vec3 dithered = color.rgb + (threshold - 0.5) / 64.0;
    return vec4(clamp(dithered, 0.0, 1.0), color.a);
}

float applyZFightingPrevention(float depth, float bias) {
    if (usePaintersAlgorithm) return depth;
    
    if (!enableZFightingPrevention) return depth;
    
    // Добавляем небольшое смещение на основе позиции фрагмента
    vec2 fragCoord = gl_FragCoord.xy;
    float pattern = sin(fragCoord.x * 0.01) * cos(fragCoord.y * 0.01);
    float fragmentBias = pattern * bias * 0.5;
    
    return depth + bias + fragmentBias;
}

void main() {
    // Применяем аффинное текстурирование
    vec2 finalTexCoord = useAffineTexturing ? 
                        affineTextureCoord(TexCoord, ClipSpacePos, ScreenPos) : 
                        TexCoord;
    
    // Читаем индекс из текстуры
    vec4 texValue = texture(indexTexture, finalTexCoord);
    float index = texValue.r;
    
    // Читаем цвет из палитры
    vec4 color = texture(paletteTexture, index);
    
    // Применяем дизеринг
    color = applyDithering(color, gl_FragCoord.xy);
    
    // ОБРАБОТКА ГЛУБИНЫ
    if (usePaintersAlgorithm) {
        // Painter's algorithm: игнорируем Z-буфер
        #if SUPPORTS_FRAG_DEPTH
            gl_FragDepth = 0.5 + depthOffset;
        #endif
    }
    
    FragColor = color;
}