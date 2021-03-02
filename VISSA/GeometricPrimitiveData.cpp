#include "GeometricPrimitiveData.h"

GLfloat Primitives::Cube::TexturedVertexData[] = {

	//position					normals					texcoords

	//FACE 1 - F
	-50.0f, -50.0f, 50.0f,		0.0f, 0.0f, 1.0f,		0.0f, 0.0f,	//bottom left	0
	50.0f, -50.0f,	50.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, //bottom right	1
	-50.0f,	50.0f, 50.0f,		0.0f, 0.0f, 1.0f,		0.0f, 1.0f, //top left		2
	50.0f, 50.0f, 50.0f,		0.0f, 0.0f, 1.0f,		1.0f, 1.0f, //top right		3
	//FACE 2 - R
	50.0f, -50.0f,	50.0f,		1.0f, 0.0f, 0.0f,		0.0f, 0.0f, //bottom left	4
	50.0f, -50.0f,	-50.0f,		1.0f, 0.0f, 0.0f,		1.0f, 0.0f, //bottom right	5
	50.0f, 50.0f, 50.0f,		1.0f, 0.0f, 0.0f,		0.0f, 1.0f, //top left		6	
	50.0f, 50.0f, -50.0f,		1.0f, 0.0f, 0.0f,		1.0f, 1.0f, //top right		7
	//FACE 3 - B
	50.0f, -50.0f,	-50.0f,		0.0f, 0.0f, -1.0f,		1.0f, 0.0f, //bottom left	8
	-50.0f, -50.0f, -50.0f,		0.0f, 0.0f, -1.0f,		0.0f, 0.0f, //bottom right	9
	50.0f, 50.0f, -50.0f,		0.0f, 0.0f, -1.0f,		1.0f, 1.0f, //top left		10
	-50.0f,	50.0f, -50.0f,		0.0f, 0.0f, -1.0f,		0.0f, 1.0f, //top right		11
	//FACE 4 - L
	-50.0f, -50.0f, -50.0f,		-1.0f, 0.0f, 0.0f,		1.0f, 0.0f, //bottom left	12
	-50.0f, -50.0f, 50.0f,		-1.0f, 0.0f, 0.0f,		0.0f, 0.0f, //bottom right	13
	-50.0f,	50.0f, -50.0f,		-1.0f, 0.0f, 0.0f,		1.0f, 1.0f, //top left		14
	-50.0f,	50.0f, 50.0f,		-1.0f, 0.0f, 0.0f,		0.0f, 1.0f, //top right		15
	//FACE 5 - TOP
	-50.0f,	50.0f, 50.0f,		0.0f, 1.0f, 0.0f,		0.0f, 0.0f,	//bottom left	16
	50.0f, 50.0f, 50.0f,		0.0f, 1.0f, 0.0f,		1.0f, 0.0f,	//bottom right	17
	-50.0f,	50.0f, -50.0f,		0.0f, 1.0f, 0.0f,		0.0f, 1.0f,	//top left		18
	50.0f, 50.0f, -50.0f,		0.0f, 1.0f, 0.0f,		1.0f, 1.0f,	//top right		19
	//FACE 6 - BOTTOM
	-50.0f,	-50.0f, -50.0f,		0.0f, -1.0f, 0.0f,		0.0f, 1.0f,	//bottom left	20
	50.0f, -50.0f, -50.0f,		0.0f, -1.0f, 0.0f,		1.0f, 1.0f,	//bottom right	21
	-50.0f,	-50.0f, 50.0f,		0.0f, -1.0f, 0.0f,		0.0f, 0.0f,	//top left		22
	50.0f, -50.0f, 50.0f,		0.0f, -1.0f, 0.0f,		1.0f, 0.0f	//top right		23
};

GLuint Primitives::Cube::TexturedIndexData[] = {

	// Front Face
	0,1,2,
	2,1,3,
	// Right Face
	4, 5, 6,
	6, 5, 7,
	// Back Face
	8,9,10,
	10,9,11,
	// Left Face
	12,13,14,
	14,13,15,
	// Top Face
	16,17,18,
	18,17,19,
	// Bottom Face
	20,21,22,
	22,21,23
};