#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "opengl.h"
#include "shader.h"

enum {
    INFO_LOG_SIZE = 1024,
};

int
shader_compile_source(int type, const char* source, long length)
{
    assert(source != NULL);

    if (type == SHADER_TYPE_VERTEX) {
        type = GL_VERTEX_SHADER;
    } else if (type == SHADER_TYPE_FRAGMENT) {
        type = GL_FRAGMENT_SHADER;
    } else {
        fprintf(stderr, "invalid shader type: %d\n", type);
        return 0;
    }

    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, (const int*)&length);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        char info_log[INFO_LOG_SIZE] = { 0 };
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, NULL, info_log);

        fprintf(stderr, "failed to compile shader source:\n%s\n", info_log);
        return 0;
    }

    return shader;
}

int
shader_link_program(int vertex_shader, int fragment_shader)
{
    int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        char info_log[INFO_LOG_SIZE] = { 0 };
        glGetProgramInfoLog(program, INFO_LOG_SIZE, NULL, info_log);

        fprintf(stderr, "failed to link shader program:\n%s\n", info_log);
        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);
        return 0;
    }

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    return program;
}
