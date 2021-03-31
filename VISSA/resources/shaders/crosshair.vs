#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 TexCoords;

uniform mat4 world;
uniform mat4 orthoProjection;

void main()
{
	//gl_Position = projection *view *world * vec4(aPos, 1.0f); // classic transform chain
	gl_Position = orthoProjection *world * vec4(aPos, 1.0f); // no view space transform here!
	TexCoords = aTexCoord;
}