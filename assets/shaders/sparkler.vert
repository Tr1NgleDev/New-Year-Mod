#version 330 core

layout(location = 0) in vec4 vert;
layout(location = 1) in float norm;

out float normComps;
out vec4 gsVert;

// model view matrix
uniform float MV[25];

vec4 Mat5_multiply(in float m[25], in vec4 v, in float finalComp)
{
	return vec4(
		m[0*5+0] * v[0] + m[1*5+0] * v[1] + m[2*5+0] * v[2] + m[3*5+0] * v[3] + m[4*5+0] * finalComp,
		m[0*5+1] * v[0] + m[1*5+1] * v[1] + m[2*5+1] * v[2] + m[3*5+1] * v[3] + m[4*5+1] * finalComp,
		m[0*5+2] * v[0] + m[1*5+2] * v[1] + m[2*5+2] * v[2] + m[3*5+2] * v[3] + m[4*5+2] * finalComp,
		m[0*5+3] * v[0] + m[1*5+3] * v[1] + m[2*5+3] * v[2] + m[3*5+3] * v[3] + m[4*5+3] * finalComp
	);
}

void main()
{
	// multiply the vertex by MV
	vec4 result = Mat5_multiply(MV, vert, 1.0);

	gl_Position = result;
	gsVert = vert;
	normComps = norm;
}
