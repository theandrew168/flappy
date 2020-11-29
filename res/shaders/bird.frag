#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_uniform_buffer_object : require

layout (location = 0) out vec4 color;

in DATA {
	vec2 tc;
} fs_in;

uniform sampler2D tex;

void main() {
	color = texture(tex, fs_in.tc);
	if (color.w < 1.0) {
		discard;
	}
}