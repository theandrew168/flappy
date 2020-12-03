#version 330 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_texcoord;

out vec2 v_texcoord;

uniform float u_angle;

void main() {
    mat2 rotate = mat2(cos(u_angle), -sin(u_angle),
                       sin(u_angle),  cos(u_angle));
    gl_Position = vec4(0.75 * rotate * a_position.xy, 0.0, 1.0);
    v_texcoord = a_texcoord;
}
