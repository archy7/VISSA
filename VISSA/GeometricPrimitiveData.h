#pragma once

#include "glad/glad.h"

namespace Primitives {

	struct Cube {
		/*
			Vertex Data for a cube with 1m³ volume
		*/
		static GLfloat TexturedVertexData[192];

		/*
			Index Data for a textured cube
		*/
		static GLuint TexturedIndexData[36];
	};
	

	//GLfloat ColorCubeVertexData[] = {

	//	//position					

	//	//FACE 1 - F
	//	-50.0f, -50.0f, 50.0f,
	//	50.0f, -50.0f,	50.0f,
	//	50.0f,	50.0f, 50.0f,
	//	50.0f, 50.0f, 50.0f,
	//	//FACE 2 - R
	//	50.0f, -50.0f,	50.0f,
	//	50.0f, -50.0f,	-50.0f,
	//	50.0f, 50.0f, 50.0f,
	//	50.0f, 50.0f, -50.0f,
	//	//FACE 3 - B
	//	50.0f, -50.0f,	-50.0f,
	//	-50.0f, -50.0f, -50.0f,
	//	50.0f, 50.0f, -50.0f,
	//	-50.0f,	50.0f, -50.0f,
	//	//FACE 4 - L
	//	-50.0f, -50.0f, -50.0f,
	//	-50.0f, -50.0f, 50.0f,
	//	-50.0f,	50.0f, -50.0f,
	//	-50.0f,	50.0f, 50.0f,
	//	//FACE 5 - TOP
	//	-50.0f,	50.0f, 50.0f,
	//	50.0f, 50.0f, 50.0f,
	//	-50.0f,	50.0f, -50.0f,
	//	50.0f, 50.0f, -50.0f,
	//	//FACE 6 - BOTTOM
	//	-50.0f,	-50.0f, -50.0f,
	//	50.0f, -50.0f, -50.0f,
	//	-50.0f,	-50.0f, 50.0f,
	//	50.0f, -50.0f, 50.0f,
	//};


	
}