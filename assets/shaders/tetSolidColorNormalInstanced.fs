#version 430 core

in float fsLighting;
in flat int fsInstanceID;

uniform vec4 inColor;

// Ouput data
layout (location=0) out vec4 color;

struct InstanceData
{
	float[25] model;
};
layout(std430, binding = 0) readonly buffer instanceData
{
    InstanceData data[];
};

void main()
{
	color = vec4(inColor.rgb * (fsLighting / 4 + 0.75), inColor.a);
}