#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;
uniform int screenWidth;
uniform int screenHeight;

// Улучшенная матрица Байера 8x8 для лучшего дизеринга
float bayerDither8x8(vec2 position) {
    int x = int(mod(position.x, 8.0));
    int y = int(mod(position.y, 8.0));
    
    const float bayerMatrix8x8[64] = float[](
        0.0/64.0, 32.0/64.0, 8.0/64.0, 40.0/64.0, 2.0/64.0, 34.0/64.0, 10.0/64.0, 42.0/64.0,
        48.0/64.0, 16.0/64.0, 56.0/64.0, 24.0/64.0, 50.0/64.0, 18.0/64.0, 58.0/64.0, 26.0/64.0,
        12.0/64.0, 44.0/64.0, 4.0/64.0, 36.0/64.0, 14.0/64.0, 46.0/64.0, 6.0/64.0, 38.0/64.0,
        60.0/64.0, 28.0/64.0, 52.0/64.0, 20.0/64.0, 62.0/64.0, 30.0/64.0, 54.0/64.0, 22.0/64.0,
        3.0/64.0, 35.0/64.0, 11.0/64.0, 43.0/64.0, 1.0/64.0, 33.0/64.0, 9.0/64.0, 41.0/64.0,
        51.0/64.0, 19.0/64.0, 59.0/64.0, 27.0/64.0, 49.0/64.0, 17.0/64.0, 57.0/64.0, 25.0/64.0,
        15.0/64.0, 47.0/64.0, 7.0/64.0, 39.0/64.0, 13.0/64.0, 45.0/64.0, 5.0/64.0, 37.0/64.0,
        63.0/64.0, 31.0/64.0, 55.0/64.0, 23.0/64.0, 61.0/64.0, 29.0/64.0, 53.0/64.0, 21.0/64.0
    );
    
    return bayerMatrix8x8[x + y * 8];
}

// Квантование до RGB555 с дизерингом
vec3 applyDithering(vec3 color, vec2 pixelCoord) {
    // Получаем значение из матрицы Байера
    float ditherValue = bayerDither8x8(pixelCoord);
    
    // Масштабируем цвет из [0,1] в [0,31] для RGB555
    vec3 scaledColor = color * 31.0;
    
    // Добавляем дизеринг (вычитаем 0.5 чтобы центрировать вокруг 0)
    vec3 dithered = scaledColor + (ditherValue - 0.5);
    
    // Квантуем до целых чисел и ограничиваем диапазон
    vec3 quantized = floor(dithered + 0.5);
    quantized = clamp(quantized, 0.0, 31.0);
    
    // Возвращаем в диапазон [0,1]
    return quantized / 31.0;
}

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    // Получаем координаты пикселя в экранном пространстве
    vec2 pixelCoord = vec2(gl_FragCoord.x, gl_FragCoord.y);
    
    // Применяем дизеринг и квантование
    vec3 finalColor = applyDithering(texColor.rgb, pixelCoord);
    
    FragColor = vec4(finalColor, texColor.a);
}