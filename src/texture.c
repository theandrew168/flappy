#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "opengl.h"
#include "texture.h"

unsigned int
texture_create(int format, long width, long height, const unsigned char* pixels)
{
    int internal_format = 0;
    if (format == TEXTURE_FORMAT_RGB) {
        format = GL_RGB;
        internal_format = GL_RGB8;
    } else if (format == TEXTURE_FORMAT_RGBA) {
        format = GL_RGBA;
        internal_format = GL_RGBA8;
    } else {
        fprintf(stderr, "invalid texture format: %d\n", format);
        return 0;
    }

    unsigned int tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}
