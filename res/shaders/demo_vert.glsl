#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

uniform mat4 u_transform;

void main() {
    gl_Position = u_transform * vec4(a_position, 1.0f);
    v_texcoord = a_texcoord;
}
