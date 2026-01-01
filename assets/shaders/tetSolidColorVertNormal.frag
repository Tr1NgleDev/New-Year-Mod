#version 330 core

in vec4 fsVertPos;
in vec4 fsNormal;

out vec4 color;

uniform vec4 lightDir;
uniform vec4 inColor;

void main()
{
	float light = max(dot(fsNormal, lightDir), 0.0) * 0.9 + 0.1;

	color = vec4(inColor.rgb * light, inColor.a);
}
