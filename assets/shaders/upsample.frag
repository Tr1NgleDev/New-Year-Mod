#version 450 core

layout (location = 0) out vec3 upsample;

in vec2 uv;

uniform sampler2D curPass;
uniform vec4 curPass_size;
const float filterRadius = 0.006;

void main()
{
	float x = filterRadius;
	float y = filterRadius;

	vec3 a = texture(curPass, vec2(uv.x - x, uv.y + y)).rgb;
	vec3 b = texture(curPass, vec2(uv.x,     uv.y + y)).rgb;
	vec3 c = texture(curPass, vec2(uv.x + x, uv.y + y)).rgb;

	vec3 d = texture(curPass, vec2(uv.x - x, uv.y)).rgb;
	vec3 e = texture(curPass, vec2(uv.x,     uv.y)).rgb;
	vec3 f = texture(curPass, vec2(uv.x + x, uv.y)).rgb;

	vec3 g = texture(curPass, vec2(uv.x - x, uv.y - y)).rgb;
	vec3 h = texture(curPass, vec2(uv.x,     uv.y - y)).rgb;
	vec3 i = texture(curPass, vec2(uv.x + x, uv.y - y)).rgb;

	upsample.rgb = e*4.0;
	upsample.rgb += (b+d+f+h)*2.0;
	upsample.rgb += (a+c+g+i);
	upsample.rgb *= 1.0 / 16.0;
}
