#version 450 core

layout(location = 0) out vec4 color;

in vec2 uv;

uniform sampler2D prevPassGroup; // same as source for the first pass group, otherwise is the texture of the last pass of the prev group
uniform sampler2D source;
uniform sampler2D sourceDepth;
uniform vec4 sourceSize; // w, h, 1/w, 1/h

void main()
{
	vec4 col = texture(prevPassGroup, uv);
	float brightness = dot(col.rgb, vec3(0.7126, 0.8152, 0.7722));
	color.rgb = col.rgb * pow(min(brightness, 1.0), 4.0);
	color.a = col.a;
}