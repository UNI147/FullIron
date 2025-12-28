#ifndef LOWRESFRAMEBUFFER_H
#define LOWRESFRAMEBUFFER_H

#include "glad.h"
#include <glm/glm.hpp>

class LowResFramebuffer {
public:
    LowResFramebuffer(int internalWidth = 320, int internalHeight = 240);
    ~LowResFramebuffer();
    
    void beginRender();
    void endRender();
    void renderToScreen(int screenWidth, int screenHeight);
    
    void setIntegerScaling(bool enable) { useIntegerScaling = enable; }
    bool getIntegerScaling() const { return useIntegerScaling; }
    
    GLuint getFramebufferID() const { return FBO; }
    GLuint getTextureID() const { return textureID; }
    
private:
    GLuint FBO;
    GLuint textureID;
    GLuint RBO;
    int internalWidth, internalHeight;
    
    GLuint quadVAO, quadVBO;
    GLuint screenShaderProgram;
    bool useIntegerScaling;
    
    void setupQuad();
    void setupScreenShader();
};

#endif