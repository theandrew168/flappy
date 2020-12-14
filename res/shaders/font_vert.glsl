#version 330 core

layout(location = 0) in vec2 a_position;

uniform float u_layer;
uniform mat4 u_model;
uniform mat4 u_projection;

void main() {
    gl_Position = u_projection * u_model * vec4(a_position, u_layer, 1.0f);
}
