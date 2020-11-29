#ifndef FLAPPY_SHADERS_DEMO_VERT_H_INCLUDED
#define FLAPPY_SHADERS_DEMO_VERT_H_INCLUDED

#include "shader.h"

static const struct shader SHADER_DEMO_VERT = {
    .type = SHADER_TYPE_VERTEX,
    .length = 239,
    .source =
        "#version 330\n"
        "layout(location = 0) in vec2 point;\n"
        "uniform float angle;\n"
        "void main() {\n"
        "    mat2 rotate = mat2(cos(angle), -sin(angle),\n"
        "                       sin(angle), cos(angle));\n"
        "    gl_Position = vec4(0.75 * rotate * point, 0.0, 1.0);\n"
        "}\n",
};

#endif
