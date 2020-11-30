#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_uniform_buffer_object : require

layout (location = 0) out vec4 color;

in DATA {
	vec2 tc;
	vec3 position;
} fs_in;

uniform vec2 bird;
uniform sampler2D tex;

void main() {
	color = texture(tex, fs_in.tc);
	color *= 3.0 / (length(bird - fs_in.position.xy) + 2.5) + 0.3;
}
