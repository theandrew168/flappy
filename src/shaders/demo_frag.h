#ifndef FLAPPY_SHADERS_DEMO_FRAG_H_INCLUDED
#define FLAPPY_SHADERS_DEMO_FRAG_H_INCLUDED

#include "shader.h"

static const struct shader SHADER_DEMO_FRAG = {
    .type = SHADER_TYPE_FRAGMENT,
    .length = 81,
    .source =
        "#version 330\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(1, 0.15, 0.15, 0);\n"
        "}\n",
};

#endif
