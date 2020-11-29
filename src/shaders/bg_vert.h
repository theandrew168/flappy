#ifndef FLAPPY_SHADERS_BG_VERT_H_INCLUDED
#define FLAPPY_SHADERS_BG_VERT_H_INCLUDED

#include "shader.h"

static const struct shader SHADER_BG_VERT = {
    .type = SHADER_TYPE_VERTEX,
    .length = 423,
    .source =
        "#version 330 core\n"
        "#extension GL_ARB_explicit_attrib_location : require\n"
        "#extension GL_ARB_uniform_buffer_object : require\n"
        "\n"
        "layout (location = 0) in vec4 position;\n"
        "layout (location = 1) in vec2 tc;\n"
        "\n"
        "uniform mat4 pr_matrix;\n"
        "uniform mat4 vw_matrix;\n"
        "\n"
        "out DATA {\n"
        "    vec2 tc;\n"
        "    vec3 position;\n"
        "} vs_out;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = pr_matrix * vw_matrix * position;\n"
        "    vs_out.tc = tc;\n"
        "    vs_out.position = vec3(vw_matrix * position);\n"
        "}\n",
};

#endif
