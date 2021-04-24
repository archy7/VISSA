#pragma once

#include "glad/glad.h"

#include "glm/glm.hpp"

namespace Primitives {
	/*
		For VISSA, 1 unit in OpenGL = 1cm, thus 100.0f = 1m.
		The following primitive vertices are specified in forms of 1m.

		Cube: 1m³
		Sphere: tightly fits a cube like defined above.
		Plane: 1m²

		All primitives are axis aligned by default.
	*/

	struct Cube {
		/*
			Vertex Data for a cube with 1m³ volume
		*/
		static GLfloat VertexData[192];	// 8 floats/Vertex, 4 vertices/face, 6 faces = 192 floats

		/*
			Index Data for a textured cube, using GL_TRIANGLES
		*/
		static GLuint IndexData[36];	// 6 indices/face, 6 faces

		/*
			Vertex Data for a colored cube with 1m³ volume
		*/
		static GLfloat SimpleVertexData[24];	// 3 floats/vertex, 8 vertices

		/*
			Index Data for a colored cube, using GL_LINE_STRIP
		*/
		static GLuint SimpleIndexData[16];		// see data

		static GLfloat DefaultCubeHalfWidth;
	};
	
	struct Plane {
		/*
			Vertex Data for a plane with and area of 1m²
			By default, the plane lies flat and is faces upward
		*/
		static GLfloat VertexData[32];	// 8 floats/Vertex, 4 vertices/face = 32 floats

		/*
			Index Data for a textured plane, using GL_TRIANGLES
		*/
		static GLuint IndexData[6];	// 6 indices/face

		/*
			Vertex Data for a colored cube with 1m³ volume
		*/
		static GLfloat SimpleVertexData[12];	// 3 floats/vertex, 4 vertices

		/*
			Index Data for a colored cube, using GL_LINE_STRIP
		*/
		static GLuint SimpleIndexData[16];		// see data
	};

	struct Sphere {
		/*
			vertex data of spheres not manually defined.
			Data is generated on the fly befor being sent to the GPU
		*/
		static GLfloat* VertexData;

		static GLuint NumberOfTrianglesInSphere;
		static GLfloat SphereDefaultRadius;
	};

	struct Line {
		static GLfloat ColoredLineVertexData[6];
	};

	namespace TwoDimensional{
		struct UniformPlane {
			static GLfloat VertexData[32];
		};
	}

	namespace Specials {
		struct GridPlane {
			static GLfloat VertexData[32];	// 8 floats/Vertex, 4 vertices/face = 32 floats
		};
	}

}