#version 430 core

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

in float normComps[];
in int instanceID[];
out float fsLighting;
out flat int fsInstanceID;

// model view matrix
//uniform float MV[25];

struct InstanceData
{
	float[25] model;
};
layout(std430, binding = 0) readonly buffer instanceData
{
    InstanceData data[];
};
uniform float[25] view;

// projection matrix is handled here after the coordinates are converted to 3D
uniform mat4 P;

// the direction that the light is coming from.
// must be normalized.
uniform vec4 lightDir;

// tetrahedron edge vertex indices
const ivec2 edges[6] = ivec2[6](ivec2(0, 1), ivec2(0, 2), ivec2(0, 3), ivec2(1, 2), ivec2(1, 3), ivec2(2, 3));

void main()
{
	int instID = instanceID[0];
	fsInstanceID = instID;

	// calculate surface normal
	mat4 modelM4 = transpose(inverse(mat4(
		vec4(data[instID].model[0 * 5 + 0], data[instID].model[0 * 5 + 1], data[instID].model[0 * 5 + 2], data[instID].model[0 * 5 + 3]),
		vec4(data[instID].model[1 * 5 + 0], data[instID].model[1 * 5 + 1], data[instID].model[1 * 5 + 2], data[instID].model[1 * 5 + 3]),
		vec4(data[instID].model[2 * 5 + 0], data[instID].model[2 * 5 + 1], data[instID].model[2 * 5 + 2], data[instID].model[2 * 5 + 3]),
		vec4(data[instID].model[3 * 5 + 0], data[instID].model[3 * 5 + 1], data[instID].model[3 * 5 + 2], data[instID].model[3 * 5 + 3])
	)));
	mat4 viewM4 = mat4(
		vec4(view[0 * 5 + 0], view[0 * 5 + 1], view[0 * 5 + 2], view[0 * 5 + 3]),
		vec4(view[1 * 5 + 0], view[1 * 5 + 1], view[1 * 5 + 2], view[1 * 5 + 3]),
		vec4(view[2 * 5 + 0], view[2 * 5 + 1], view[2 * 5 + 2], view[2 * 5 + 3]),
		vec4(view[3 * 5 + 0], view[3 * 5 + 1], view[3 * 5 + 2], view[3 * 5 + 3])
	);

	vec4 normal = normalize(viewM4 * modelM4 * vec4(normComps[0], normComps[1], normComps[2], normComps[3]));

	// calculate how close the normal is to the light direction
	float lighting = dot(normal, lightDir);

	// calculate intersection between simplex and hyperplane (3D view)

	// number of intersection points
	int k = 0;
	for (int i = 0; i < 6; ++i)
	{
		if (k == 4)
		{
			break;
		}

		ivec2 e = edges[i];
		vec4 p0 = gl_in[e[0]].gl_Position;
		vec4 p1 = gl_in[e[1]].gl_Position;

		// vertices are on the same side of the hyperplane so the edge doesn't intersect
		if ((p0.w < 0.00000001 && p1.w < 0.00000001) || (p0.w > -0.00000001 && p1.w > -0.00000001))
		{
			continue;
		}

		// intersection
		float a = 0.0;
		if (abs(p1.w - p0.w) > 0.00000001)
		{
			a = (-p0.w) / (p1.w - p0.w);
		}

		vec4 pt = mix(p0, p1, a);
		gl_Position = P * vec4(pt.xyz, 1.0);
		fsLighting = lighting;

		EmitVertex();

		++k;
	}
	
	EndPrimitive();
}