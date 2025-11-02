#version 430 core

uniform vec4 inColor;

// Ouput data
layout (location=0) out vec4 color;

in vec4 fsNormal;
in vec4 fsVertNormal;
in vec4 fsTangent;
in vec4 fsVertTangent;
in vec4 fsColor;
in vec3 fsUVW;

//vec4 lightDir = normalize(vec4(0.2,1.0,0.1,0.4));

uniform float time;

void main()
{
	//color = vec4(fsUVW, 1.0);
	color = fsColor;
}