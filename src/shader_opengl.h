#ifndef FLAPPY_SHADER_OPENGL_H_INCLUDED
#define FLAPPY_SHADER_OPENGL_H_INCLUDED

#include <GL/glcorearb.h>

GLuint shader_opengl_compile(const char* source, long length);
GLuint shader_opengl_link(GLuint vertex_shader, GLuint fragment_shader);

#endif
