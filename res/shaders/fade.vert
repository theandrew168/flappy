#version 330 core
#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_uniform_buffer_object : require

void main() {
	const vec4 vertices[6] = vec4[6](vec4( 1.0, -1.0, -0.5, 1.0),
									 vec4(-1.0, -1.0, -0.5, 1.0),
									 vec4( 1.0,  1.0, -0.5, 1.0),
									 vec4( 1.0,  1.0, -0.5, 1.0),
									 vec4(-1.0, -1.0, -0.5, 1.0),
									 vec4(-1.0,  1.0, -0.5, 1.0));
	gl_Position = vertices[gl_VertexID];
}