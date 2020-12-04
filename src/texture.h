#ifndef FLAPPY_TEXTURE_H_INCLUDED
#define FLAPPY_TEXTURE_H_INCLUDED

enum texture_format {
    TEXTURE_FORMAT_UNDEFINED = 0,
    TEXTURE_FORMAT_RGB,
    TEXTURE_FORMAT_RGBA,
};

unsigned int texture_create(int format, long width, long height, const unsigned char* pixels);

#endif
