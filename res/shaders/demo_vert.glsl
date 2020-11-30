#version 330
layout(location = 0) in vec2 point;
uniform float angle;
void main() {
    mat2 rotate = mat2(cos(angle), -sin(angle),
                       sin(angle), cos(angle));
    gl_Position = vec4(0.75 * rotate * point, 0.0, 1.0);
}
