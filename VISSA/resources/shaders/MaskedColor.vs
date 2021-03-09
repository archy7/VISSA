#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

layout (std140, binding = 0) uniform Matrices{
	mat4 view;
	mat4 projection;	
};

uniform mat4 world;

void main()
{
	gl_Position = projection *view *world * vec4(aPos, 1.0f);
	TexCoords = aTexCoord;
}