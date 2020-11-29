#ifndef FLAPPY_SHADER_H_INCLUDED
#define FLAPPY_SHADER_H_INCLUDED

enum shader_type {
    SHADER_TYPE_UNDEFINED = 0,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT,
};

struct shader {
    int type;
    long length;
    char* source;
};

#endif
