#version 430 core
layout (location = 0) in vec3 position_vs;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection *view *world * vec4(position_vs, 1.0f);
}