#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "model.h"
#include "opengl.h"

// Return the number of float "components" in a vertex of a given format
static long
model_vertex_size(int format)
{
    switch (format) {
    case MODEL_FORMAT_V3F:
        return 3;
    case MODEL_FORMAT_N3F_V3F:
        return 6;
    case MODEL_FORMAT_T2F_N3F_V3F:
        return 8;
    default:
        fprintf(stderr, "Invalid model format: %d\n", format);
        return -1;
    }
}

unsigned int
model_buffer_create(int format, long count, const float* vertices)
{
    // calculate size of vertex buffer in bytes
    long size = count * model_vertex_size(format) * sizeof(float);

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
    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    long stride = model_vertex_size(format);
    switch (format) {
    case MODEL_FORMAT_T2F_N3F_V3F:
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
    case MODEL_FORMAT_N3F_V3F:
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    case MODEL_FORMAT_V3F:
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0 * sizeof(float)));
        glEnableVertexAttribArray(0);
        break;
    default:
        fprintf(stderr, "Invalid model format: %d\n", format);
        return 0;
    }

    // unbind VBO _after_ VAO in order to properly capture state?
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vao;
}
