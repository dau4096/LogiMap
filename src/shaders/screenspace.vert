/* screenspace.vert */
#version 460 core

out vec2 fragUV;


const vec2 v[4] = {
	vec2(-1.0f, -1.0f),
	vec2( 1.0f, -1.0f),
	vec2(-1.0f,  1.0f),
	vec2( 1.0f,  1.0f),
};

void main() {
	gl_Position = vec4(v[gl_VertexID].xy, 0.0f, 1.0f);
	fragUV = clamp(v[gl_VertexID], 0.0f, 1.0f);
}