#version 450

// Hardcoded: Setup triangle vertices
//vec2 positions[3] = vec2[] (
//	vec2(0.0, -0.5),
//	vec2(0.5, 0.5),
//	vec2(-0.5, 0.5)
//);

// Specified first vertex attribute
// in = takes value from a vertex buffer
layout(location = 0) in vec2 position;

void main() {

	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);	// gl_VertexIndex appears as error in VS19, but compiles fine
	gl_Position = vec4(position, 0.0, 1.0);
}