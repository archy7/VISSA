#version 430 core
layout (location = 0) in vec3 position_vs;

layout (std140, binding = 0) uniform Matrices{
	mat4 view;
	mat4 projection;	
};

uniform mat4 world;

void main()
{
	gl_Position = projection *view *world * vec4(position_vs, 1.0f);
}