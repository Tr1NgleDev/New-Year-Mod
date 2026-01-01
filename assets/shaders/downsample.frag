#version 450 core

layout (location = 0) out vec3 downsample;

in vec2 uv;

uniform sampler2D prevPass;
uniform vec4 prevPass_size;

void main()
{
	vec2 srcTexelSize = prevPass_size.zw;
	float x = srcTexelSize.x;
	float y = srcTexelSize.y;

	vec3 a = texture(prevPass, vec2(uv.x - 2*x, uv.y + 2*y)).rgb;
	vec3 b = texture(prevPass, vec2(uv.x,       uv.y + 2*y)).rgb;
	vec3 c = texture(prevPass, vec2(uv.x + 2*x, uv.y + 2*y)).rgb;

	vec3 d = texture(prevPass, vec2(uv.x - 2*x, uv.y)).rgb;
	vec3 e = texture(prevPass, vec2(uv.x,       uv.y)).rgb;
	vec3 f = texture(prevPass, vec2(uv.x + 2*x, uv.y)).rgb;

	vec3 g = texture(prevPass, vec2(uv.x - 2*x, uv.y - 2*y)).rgb;
	vec3 h = texture(prevPass, vec2(uv.x,       uv.y - 2*y)).rgb;
	vec3 i = texture(prevPass, vec2(uv.x + 2*x, uv.y - 2*y)).rgb;

	vec3 j = texture(prevPass, vec2(uv.x - x, uv.y + y)).rgb;
	vec3 k = texture(prevPass, vec2(uv.x + x, uv.y + y)).rgb;
	vec3 l = texture(prevPass, vec2(uv.x - x, uv.y - y)).rgb;
	vec3 m = texture(prevPass, vec2(uv.x + x, uv.y - y)).rgb;

	downsample.rgb = e*0.125;
	downsample.rgb += (a+c+g+i)*0.03125;
	downsample.rgb += (b+d+f+h)*0.0625;
	downsample.rgb += (j+k+l+m)*0.125;
	downsample.rgb = max(downsample.rgb, vec3(0.0001));
}
