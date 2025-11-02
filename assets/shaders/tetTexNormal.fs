#version 330 core

// Interpolated values from the vertex shaders
in vec3 fsColor;
in float fsLighting;

// Ouput data
out vec4 color;

uniform sampler2DArray image;
uniform int imageZSize;
uniform vec3 sunlightColor;

void main()
{
	vec4 col = texture(image, vec3(fsColor.xy, fsColor.z * float(imageZSize)));
	color = vec4(col.rgb * (fsLighting / 4 + 0.75) * sunlightColor, col.a);
}