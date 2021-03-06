#pragma once

#include "glad/glad.h"

namespace Primitives {
	/*
		For VISSA, 1 unit in OpenGL = 1cm, thus 100.0f = 1m.
		The following primitive vertices are specified in forms of 1m.

		Cube: 1m³
		Sphere: 1m diameter
		Plane: 1m²

		All primitives are axis aligned by default.
	*/

	struct Cube {
		/*
			Vertex Data for a cube with 1m³ volume
		*/
		static GLfloat TexturedVertexData[192];	// 8 floats/Vertex, 4 vertices/face, 6 faces = 192 floats

		/*
			Index Data for a textured cube, using GL_TRIANGLES
		*/
		static GLuint TexturedIndexData[36];	// 6 indices/face, 6 faces

		/*
			Vertex Data for a colored cube with 1m³ volume
		*/
		static GLfloat ColoredVertexData[24];	// 3 floats/vertex, 8 vertices

		/*
			Index Data for a colored cube, using GL_LINE_STRIP
		*/
		static GLuint ColoredIndexData[16];		// see data
	};
	
	struct Plane {
		/*
			Vertex Data for a plane with and area of 1m²
			By default, the plane lies flat and is faces upward
		*/
		static GLfloat TexturedVertexData[32];	// 8 floats/Vertex, 4 vertices/face = 32 floats

		/*
			Index Data for a textured plane, using GL_TRIANGLES
		*/
		static GLuint TexturedIndexData[6];	// 6 indices/face

		/*
			Vertex Data for a colored cube with 1m³ volume
		*/
		static GLfloat ColoredVertexData[12];	// 3 floats/vertex, 4 vertices

		/*
			Index Data for a colored cube, using GL_LINE_STRIP
		*/
		static GLuint ColoredIndexData[16];		// see data
	};

	struct Sphere {
		/*
			vertex data of spheres not manually defined.
			Data is generated on the fly befor being sent to the GPU
		*/
		static GLuint NumberOfTrianglesInSphere;
		static GLfloat SphereDefaultRadius;
	};

}