#version 450 core

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D p_group_pass0; // bloomTex
uniform sampler2D pppp_group; // before bloom group
uniform sampler2D source;
uniform sampler2D sourceDepth;
uniform vec4 sourceSize; // w, h, 1/w, 1/h
uniform float intensity;

void main()
{
	vec3 bloomTex = texture(p_group_pass0, uv).rgb;
	color = texture(pppp_group, uv);
	color.a = 1.0;
	color.rgb += bloomTex * 0.12 * intensity;
}
