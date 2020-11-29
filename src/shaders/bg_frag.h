#ifndef FLAPPY_SHADERS_BG_FRAG_H_INCLUDED
#define FLAPPY_SHADERS_BG_FRAG_H_INCLUDED

#include "shader.h"

static const struct shader SHADER_BG_FRAG = {
    .type = SHADER_TYPE_FRAGMENT,
    .length = 363,
    .source =
        "#version 330 core\n"
        "#extension GL_ARB_explicit_attrib_location : require\n"
        "#extension GL_ARB_uniform_buffer_object : require\n"
        "\n"
        "layout (location = 0) out vec4 color;\n"
        "\n"
        "in DATA {\n"
        "	vec2 tc;\n"
        "	vec3 position;\n"
        "} fs_in;\n"
        "\n"
        "uniform vec2 bird;\n"
        "uniform sampler2D tex;\n"
        "\n"
        "void main() {\n"
        "	color = texture(tex, fs_in.tc);\n"
        "	color *= 3.0 / (length(bird - fs_in.position.xy) + 2.5) + 0.3;\n"
        "}\n",
};

#endif
