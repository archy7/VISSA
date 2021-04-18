#version 430 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec4 color;

uniform sampler2D transparencyMask;

void main()
{
	vec4 texColor = texture(transparencyMask, TexCoords);
    if(texColor.a < 0.1)
        discard;
	
	// Set from the outside.
	FragColor = color;
	//FragColor = texColor;
}