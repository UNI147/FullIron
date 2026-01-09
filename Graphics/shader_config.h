#ifndef SHADER_CONFIG_H
#define SHADER_CONFIG_H

// Определения для шейдеров
#define USE_AFFINE_TEXTURING 1
#define USE_VERTEX_SNAPPING 1
#define USE_VERTEX_JITTER 1
#define USE_ZFIGHTING_PREVENTION 1
#define USE_PAINTERS_ALGORITHM 1

// Проверка поддержки расширений
#ifdef GL_ARB_fragment_shader
    #define SUPPORTS_FRAG_DEPTH 1
#else
    #define SUPPORTS_FRAG_DEPTH 0
#endif

#endif