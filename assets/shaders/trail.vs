#version 330 core

layout(location = 0) in vec4 vert;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 tangent;
layout(location = 3) in vec4 color;
layout(location = 4) in vec3 uvw;

// model view matrix
uniform float MV[25];

out vec4 gsNormal;
out vec4 gsVertNormal;
out vec4 gsTangent;
out vec4 gsVertTangent;
out vec4 gsColor;
out vec3 gsUVW;

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

	mat4 MVM4 = transpose(inverse(mat4(
		vec4(MV[0 * 5 + 0], MV[0 * 5 + 1], MV[0 * 5 + 2], MV[0 * 5 + 3]),
		vec4(MV[1 * 5 + 0], MV[1 * 5 + 1], MV[1 * 5 + 2], MV[1 * 5 + 3]),
		vec4(MV[2 * 5 + 0], MV[2 * 5 + 1], MV[2 * 5 + 2], MV[2 * 5 + 3]),
		vec4(MV[3 * 5 + 0], MV[3 * 5 + 1], MV[3 * 5 + 2], MV[3 * 5 + 3])
	)));
	
	vec4 result = Mat5_multiply(MV, vert, 1.0);
    vec4 resultN = normalize(MVM4 * normal);
    vec4 resultT = normalize(MVM4 * tangent);

	gsNormal = resultN;
	gsVertNormal = normal;
	gsTangent = resultT;
	gsVertTangent = tangent;
	gsColor = color;
	gsUVW = uvw;

	gl_Position = result;
}