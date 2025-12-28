#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D indexTexture;
uniform sampler1D paletteTexture;
uniform bool useDithering;
uniform vec2 resolution;

// Матрица Байера для дизеринга
float bayer4x4[16] = float[](
    0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
    12.0/16.0, 4.0/16.0, 14.0/16.0,  6.0/16.0,
    3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
    15.0/16.0, 7.0/16.0, 13.0/16.0,  5.0/16.0
);

void main() {
    // Читаем индекс из текстуры
    vec4 texValue = texture(indexTexture, TexCoord);
    float index = texValue.r;
    
    // Читаем цвет из палитры
    vec4 color = texture(paletteTexture, index);
    
    FragColor = color;
}