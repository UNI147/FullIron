#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D screenTexture;

// Честное масштабирование (ближайший сосед)
void main() {
    // Point sampling (ближайший сосед) для сохранения четких пикселей
    FragColor = texture(screenTexture, TexCoord);
}