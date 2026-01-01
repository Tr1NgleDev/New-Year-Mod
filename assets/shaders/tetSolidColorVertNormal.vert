#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 normal;

out vec4 gsVertPos;
out vec4 gsNormal;

// model view matrix
uniform float MV[25];

mat4 MV_mat4()
{
	return mat4(
		vec4(MV[0*5+0], MV[0*5+1], MV[0*5+2], MV[0*5+3]),
		vec4(MV[1*5+0], MV[1*5+1], MV[1*5+2], MV[1*5+3]),
		vec4(MV[2*5+0], MV[2*5+1], MV[2*5+2], MV[2*5+3]),
		vec4(MV[3*5+0], MV[3*5+1], MV[3*5+2], MV[3*5+3])
	);
}

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
	vec4 result = Mat5_multiply(MV, pos, 1.0);

	gl_Position = result;
	gsVertPos = pos;
	gsNormal = normalize(transpose(inverse(MV_mat4())) * normal);
}
