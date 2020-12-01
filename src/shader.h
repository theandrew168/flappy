#ifndef FLAPPY_SHADER_H_INCLUDED
#define FLAPPY_SHADER_H_INCLUDED

enum shader_type {
    SHADER_TYPE_UNDEFINED = 0,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT,
};

int shader_compile_source(int type, const char* source, long length);
int shader_link_program(int vertex_shader, int fragment_shader);

#endif
