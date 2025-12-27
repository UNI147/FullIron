#version 330 core
in vec2 TexCoord;
out vec4 FragColor;
uniform sampler2D texture1;

// Простое квантование до RGB555 без дизеринга
vec3 quantizeToRGB555(vec3 color) {
    vec3 scaled = color * 31.0;
    vec3 quantized = floor(scaled + 0.5);
    return quantized / 31.0;
}

void main() {
    vec4 texColor = texture(texture1, TexCoord);
    
    // Простое квантование (без дизеринга)
    vec3 finalColor = quantizeToRGB555(texColor.rgb);
    
    FragColor = vec4(finalColor, texColor.a);
}