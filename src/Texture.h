#ifndef TEXTURE_H
#define TEXTURE_H

#include "glad.h"
#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();
    
    void bind(GLuint unit = 0) const;
    GLuint getID() const { return ID; }
    
private:
    GLuint ID;
    int width, height, nrChannels;
};

#endif