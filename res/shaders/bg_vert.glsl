#version 330 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_uniform_buffer_object : require

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 tc;

layout(location = 0) uniform mat4 pr_matrix;
layout(location = 1) uniform mat4 vw_matrix;

out DATA {
	vec2 tc;
	vec3 position;
} vs_out;

void main() {
	gl_Position = pr_matrix * vw_matrix * position;
	vs_out.tc = tc;
	vs_out.position = vec3(vw_matrix * position);
}
