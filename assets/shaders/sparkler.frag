#version 330 core

in float fsLighting;
in vec4 fsVert;

uniform float progress;

out vec4 color;

const vec3 baseCol = vec3(0.07);
const vec3 brightCol = vec3(1.0f, 0.9f, 0.5f) * 1.5;
const vec3 ashCol = vec3(0.29);

const float transition = 0.1;
const float brightLen = 0.0;

void main()
{
	float y = fsVert.y;
	float p = 1.0 - mix(progress, progress + (transition + brightLen) * 2.0, progress);
	float bright = smoothstep(y, y - transition, p);
	float ash = smoothstep(y - transition - brightLen, y - transition - brightLen - transition, p);
	vec3 col = mix(mix(baseCol, brightCol, bright), ashCol, ash);
	color = vec4(col * (fsLighting * 0.25 + 0.75), 1.0);
}
