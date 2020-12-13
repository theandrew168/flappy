#version 330 core

in vec2 v_texcoord;

out vec4 FragColor;

uniform sampler2D u_texture;

void main() {
    FragColor = texture(u_texture, v_texcoord);
}
