#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "opengl.h"

unsigned int
model_buffer_create(int format, long count, const float* vertices)
{
    long size = 0;
    if (format == MODEL_FORMAT_V3F) {
        size = 3;
    } else if (format == MODEL_FORMAT_N3F_V3F) {
        size = 6;
    } else if (format == MODEL_FORMAT_T2F_N3F_V3F) {
        size = 8;
    } else {
        fprintf(stderr, "Invalid model format: %d\n", format);
        return 0;
    }

    // calculate size of vertex buffer in bytes
    size = size * sizeof(float) * count;

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vbo;
}

unsigned int
model_buffer_config(int format, int buffer)
{
    return 0;
}
