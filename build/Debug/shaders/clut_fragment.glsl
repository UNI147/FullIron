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

// Матрица Байера для дизеринга (оставляем как было)
float bayer4x4[16] = float[](
    0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
    3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
);

// АФФИННОЕ ТЕКСТУРИРОВАНИЕ
vec2 affineTextureCoord(vec2 texCoord, vec4 clipPos, vec2 screenPos) {
    if (!useAffineTexturing) {
        return texCoord;
    }
    
    // квантование
    ivec2 pixelPos = ivec2(screenPos);
    float texelPrecision = 256.0;
    
    vec2 quantizedTexCoord;
    quantizedTexCoord.x = floor(texCoord.x * texelPrecision) / texelPrecision;
    quantizedTexCoord.y = floor(texCoord.y * texelPrecision) / texelPrecision;
    
    // перспективное искажение
    float w = clipPos.w;
    float perspectiveFactor = 1.0 / max(w, 1.0);
    
    // интенсивность
    float intensity = 0.015 * perspectiveFactor;
    
    // Основные искажения
    vec2 distortion;
    distortion.x = sin(texCoord.x * 5.0 + time * 0.8) * intensity;
    distortion.y = cos(texCoord.y * 5.0 + time * 0.6) * intensity;
    
    // второстепенные искажения
    vec2 secondaryDistortion;
    secondaryDistortion.x = sin(texCoord.x * 12.0 + time * 1.5) * intensity * 0.3;
    secondaryDistortion.y = cos(texCoord.y * 12.0 + time * 1.2) * intensity * 0.3;
    
    // Комбинируем искажения
    vec2 finalCoord = quantizedTexCoord + distortion + secondaryDistortion;
    
    // Эффект "плавающих" текстур
    float swimSpeed = 0.12;
    float swimAmount = 0.0012;
    
    // Зависимость от глубины
    float depthFactor = 1.0 - min(ClipSpacePos.w * 0.15, 1.0);
    swimAmount *= depthFactor;
    
    finalCoord.x += sin(time * swimSpeed + FragPos.x * 3.0) * swimAmount;
    finalCoord.y += cos(time * swimSpeed * 0.8 + FragPos.y * 3.0) * swimAmount;
    
    // ТОЧЕЧНАЯ ВЫБОРКА
    ivec2 texSize = textureSize(indexTexture, 0);
    ivec2 texelPos = ivec2(finalCoord * vec2(texSize));
    finalCoord = vec2(texelPos) / vec2(texSize);
    
    return finalCoord;
}

// Дизеринг
vec4 applyDithering(vec4 color, vec2 fragCoord) {
    if (!useDithering) return color;
    
    ivec2 pixelPos = ivec2(fragCoord);
    
    int x = pixelPos.x & 7;
    int y = pixelPos.y & 7;
    
    // матрица Байера 8x8
    float bayer8x8[64] = float[](
        0.0/64.0, 32.0/64.0, 8.0/64.0, 40.0/64.0, 2.0/64.0, 34.0/64.0, 10.0/64.0, 42.0/64.0,
        48.0/64.0, 16.0/64.0, 56.0/64.0, 24.0/64.0, 50.0/64.0, 18.0/64.0, 58.0/64.0, 26.0/64.0,
        12.0/64.0, 44.0/64.0, 4.0/64.0, 36.0/64.0, 14.0/64.0, 46.0/64.0, 6.0/64.0, 38.0/64.0,
        60.0/64.0, 28.0/64.0, 52.0/64.0, 20.0/64.0, 62.0/64.0, 30.0/64.0, 54.0/64.0, 22.0/64.0,
        3.0/64.0, 35.0/64.0, 11.0/64.0, 43.0/64.0, 1.0/64.0, 33.0/64.0, 9.0/64.0, 41.0/64.0,
        51.0/64.0, 19.0/64.0, 59.0/64.0, 27.0/64.0, 49.0/64.0, 17.0/64.0, 57.0/64.0, 25.0/64.0,
        15.0/64.0, 47.0/64.0, 7.0/64.0, 39.0/64.0, 13.0/64.0, 45.0/64.0, 5.0/64.0, 37.0/64.0,
        63.0/64.0, 31.0/64.0, 55.0/64.0, 23.0/64.0, 61.0/64.0, 29.0/64.0, 53.0/64.0, 21.0/64.0
    );
    
    float threshold = bayer8x8[x + y * 8];
    
    float ditherStrength = 0.02;
    
    // Дизеринг для каждого канала отдельно
    vec3 dithered = color.rgb;
    dithered.r += (threshold - 0.5) * ditherStrength;
    dithered.g += (threshold - 0.5) * ditherStrength * 1.1;
    dithered.b += (threshold - 0.5) * ditherStrength * 0.9;
    
    // Резкое квантование
    float colorSteps = 16.0;
    dithered = floor(dithered * colorSteps + 0.5) / colorSteps;
    
    return vec4(clamp(dithered, 0.0, 1.0), color.a);
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
    if (useDithering) {
        color = applyDithering(color, gl_FragCoord.xy);
    }
    
    FragColor = color;
}