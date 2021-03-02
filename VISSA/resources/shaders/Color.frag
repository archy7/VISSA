#version 430 core
out vec4 FragColor;

uniform vec4 color;

void main()
{
	// Set from the outside.
	FragColor = color;
}