#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>>

#include "generalGL.h"
#include "Shader.h"
#include "Camera.h"
#include "Window.h"
#include "GUI.h"
#include "GeometricPrimitiveData.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct TriangularFace {
	struct Vertex{
		glm::vec3 vec3Position;
		glm::vec3 vec3Normal;
		glm::vec2 vec2UVs;
	};

	Vertex vertex1;
	Vertex vertex2;
	Vertex vertex3;
};

Renderer::Renderer() :
	// projection members
	m_fOrthoLeft(0.0f),
	m_fOrthoRight(1280.0f),
	m_fOrthoBottom(0.0f),
	m_fOrthoTop(720.0f),
	m_fNearPlane(0.1f),
	m_fFarPlane(1000.0f),
	m_vec4fClearColor(0.3f, 0.3f, 0.3f, 1.0f) // a light grey tone
{
	
}

Renderer::~Renderer()
{
	FreeGPUResources();
}

void Renderer::InitRenderer()
{
	LoadShaders();
	glAssert();
	LoadTextures();
	glAssert();
	InitUniformBuffers();
	glAssert();
	LoadPrimitivesToGPU();
	glAssert();
	SetInitialOpenGLState();
	glAssert();
}

/*
   Create a triangular facet approximation to a sphere
   Return the number of facets created.
   The number of facets will be 8 * (4^iterations)
*/
Renderer::SphereTrianglesGenerationResult Renderer::GenerateSphereVertexData(float fRadius, int iterations)
{
	// building and populating the result struct
	SphereTrianglesGenerationResult tResultStruct;
	tResultStruct.m_uiNumberOfTriangles = 8u * static_cast<size_t>(std::pow(4.0f, iterations));
	//tResultStruct.m_pTriangleData = new TriangularFace[tResultStruct.m_uiNumberOfTriangles];

	TriangularFace* pReadBuffer = new TriangularFace[tResultStruct.m_uiNumberOfTriangles];
	TriangularFace* pWriteBuffer = new TriangularFace[tResultStruct.m_uiNumberOfTriangles];

	// within a unit sphere, these positions translate to the following corners of an octahedron:
	glm::vec3 normalizedStartingPoints[6] = {
			glm::vec3(0.0f,1.0f,0.0f),		// TOP
			glm::vec3(0.0f,-1.0f,0.0f),		// BOTTON
			glm::normalize(glm::vec3(-1.0f,0.0f,1.0f)),	// 4 corners around its "belt" or equator
			glm::normalize(glm::vec3(1.0f,0.0f,1.0f)),
			glm::normalize(glm::vec3(1.0f,0.0f,-1.0f)),
			glm::normalize(glm::vec3(-1.0f,0.0f,-1.0f))
	};
	
	auto UVsFromPosition = [&](const glm::vec3& normalizedSpherePoint) {
		glm::vec2 uvResult;
		uvResult.x = normalizedSpherePoint.x * 0.5f + 0.5f;	// http://www.mvps.org/directx/articles/spheremap.htm
		uvResult.y = normalizedSpherePoint.y * 0.5f + 0.5f;	// since the points are normalized, they are equal to normal vectors
		return uvResult;
	};

	auto ConstructVertexFromNormalizedSpherePoint = [&](const glm::vec3& normalizedSpherePoint) {
		TriangularFace::Vertex newStartingVertex;
		newStartingVertex.vec3Position = normalizedSpherePoint * fRadius;		// just assume position
		newStartingVertex.vec3Normal = normalizedSpherePoint;		// can assume position for normal vector, because we are dealing with points on a unit sphere.
		newStartingVertex.vec2UVs = UVsFromPosition(normalizedSpherePoint);
		return newStartingVertex;
	};

	// Create the level 0 object, an octahedron. 8 triangles with 3 vertices each
	// TOP HALF of octahedron
	// first triangle
	pWriteBuffer[0].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[0]);
	pWriteBuffer[0].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[3]);
	pWriteBuffer[0].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[4]);

	// second triangle
	pWriteBuffer[1].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[0]);
	pWriteBuffer[1].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[4]);
	pWriteBuffer[1].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[5]);

	// third triangle
	pWriteBuffer[2].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[0]);
	pWriteBuffer[2].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[5]);
	pWriteBuffer[2].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[2]);

	// fourth triangle
	pWriteBuffer[3].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[0]);
	pWriteBuffer[3].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[2]);
	pWriteBuffer[3].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[3]);

	// BOTTOM HALF of octahedron
	// first triangle
	pWriteBuffer[4].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[1]);
	pWriteBuffer[4].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[4]);
	pWriteBuffer[4].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[3]);

	// second triangle
	pWriteBuffer[5].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[1]);
	pWriteBuffer[5].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[5]);
	pWriteBuffer[5].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[4]);

	// third triangle
	pWriteBuffer[6].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[1]);
	pWriteBuffer[6].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[2]);
	pWriteBuffer[6].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[5]);

	// fourth triangle
	pWriteBuffer[7].vertex1 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[1]);
	pWriteBuffer[7].vertex2 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[3]);
	pWriteBuffer[7].vertex3 = ConstructVertexFromNormalizedSpherePoint(normalizedStartingPoints[2]);

	glm::vec3 newPosition_1_2, newPosition_2_3, newPosition_3_1;

	/* Bisect each edge and move to the surface of a unit sphere */
	

	for (int iCurrentIteration = 0; iCurrentIteration < iterations; iCurrentIteration++) 
	{
		int iNumberTrianglesPreviousIteration = 8 * static_cast<int>(std::pow(4.0f, iCurrentIteration)); // = 8, 32, 128 for currentIteration = 0, 1, 2
		std::swap(pWriteBuffer, pReadBuffer);
		int iCurrentTriangleWritingIndex = 0;

		for (int iCurrentTriangleReadingIndex = 0; iCurrentTriangleReadingIndex < iNumberTrianglesPreviousIteration; iCurrentTriangleReadingIndex++) 
		{
			// create the new point between points 1 and 2
			newPosition_1_2 = pReadBuffer[iCurrentTriangleReadingIndex].vertex1.vec3Position * 0.5f + pReadBuffer[iCurrentTriangleReadingIndex].vertex2.vec3Position * 0.5f;
			// create the new point between points 2 and 3
			newPosition_2_3 = pReadBuffer[iCurrentTriangleReadingIndex].vertex2.vec3Position * 0.5f + pReadBuffer[iCurrentTriangleReadingIndex].vertex3.vec3Position * 0.5f;
			// create the new point between points 3 and 1
			newPosition_3_1 = pReadBuffer[iCurrentTriangleReadingIndex].vertex3.vec3Position * 0.5f + pReadBuffer[iCurrentTriangleReadingIndex].vertex1.vec3Position * 0.5f;

			// normalizing the new points so they lie on a unit sphere
			const glm::vec3 newPoint_1_2_normalized = glm::normalize(newPosition_1_2);
			const glm::vec3 newPoint_2_3_normalized = glm::normalize(newPosition_2_3);
			const glm::vec3 newPoint_3_1_normalized = glm::normalize(newPosition_3_1);

			/*

				For every old face/triangle, we subdivide it into 4 new ones.
				The first 3 triangles each re-use one of the original triangle's corners.

						       /\
							  /  \
							 /    \
							/      \
						   /        \
						  /__________\
						 /\          /\
						/  \        /  \
					   /    \      /    \
					  /      \    /      \
					 /        \  /        \
					/__________\/__________\
					
			*/			

			// the first new triangle
			// vertex 1
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex1 = ConstructVertexFromNormalizedSpherePoint(glm::normalize(pReadBuffer[iCurrentTriangleReadingIndex].vertex1.vec3Position));
			// vertex 2
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex2 = ConstructVertexFromNormalizedSpherePoint(newPoint_1_2_normalized);
			// vertex 3
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex3 = ConstructVertexFromNormalizedSpherePoint(newPoint_3_1_normalized);
			iCurrentTriangleWritingIndex++;

			// the second new triangle
			// vertex 1
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex1 = ConstructVertexFromNormalizedSpherePoint(newPoint_1_2_normalized);
			// vertex 2
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex2 = ConstructVertexFromNormalizedSpherePoint(glm::normalize(pReadBuffer[iCurrentTriangleReadingIndex].vertex2.vec3Position));
			// vertex 3
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex3 = ConstructVertexFromNormalizedSpherePoint(newPoint_2_3_normalized);
			iCurrentTriangleWritingIndex++;

			// the third new triangle
			// vertex 1
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex1 = ConstructVertexFromNormalizedSpherePoint(newPoint_2_3_normalized);
			// vertex 2
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex2 = ConstructVertexFromNormalizedSpherePoint(glm::normalize(pReadBuffer[iCurrentTriangleReadingIndex].vertex3.vec3Position));
			// vertex 3
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex3 = ConstructVertexFromNormalizedSpherePoint(newPoint_3_1_normalized);
			iCurrentTriangleWritingIndex++;

			// the fourth new triangle, made up entirely from the new points. now residing in memory where the original triangle used to live.
			// vertex 1
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex1 = ConstructVertexFromNormalizedSpherePoint(newPoint_1_2_normalized);
			// vertex 2
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex2 = ConstructVertexFromNormalizedSpherePoint(newPoint_2_3_normalized);
			// vertex 3
			pWriteBuffer[iCurrentTriangleWritingIndex].vertex3 = ConstructVertexFromNormalizedSpherePoint(newPoint_3_1_normalized);
			iCurrentTriangleWritingIndex++;
		}

		
	}

	tResultStruct.m_pTriangleData = pWriteBuffer;
	delete[] pReadBuffer;
	return tResultStruct;
}

void Renderer::LoadShaders()
{
	// A simple color shader
	Shader tColorShader("resources/shaders/Color.vs", "resources/shaders/Color.frag");
	m_tColorShader = tColorShader;
	assert(m_tColorShader.IsInitialized());

	// A texture shader
	Shader tTextureShader("resources/shaders/FlatTexture.vs", "resources/shaders/FlatTexture.frag");
	m_tTextureShader = tTextureShader;
	assert(m_tTextureShader.IsInitialized());
}

void Renderer::LoadTextures()
{
	// texture 1
	// ---------
	GLuint& rTextureUnit1 = m_uiTexture1;
	glGenTextures(1, &rTextureUnit1);
	glBindTexture(GL_TEXTURE_2D, rTextureUnit1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load("resources/textures/container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);


	// texture 2
	// ---------
	GLuint& rTextureUnit2 = m_uiTexture2;
	glGenTextures(1, &rTextureUnit2);
	glBindTexture(GL_TEXTURE_2D, rTextureUnit2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load("resources/textures/awesomeface.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

void Renderer::LoadPrimitivesToGPU()
{
	// textured cube
	{
		GLuint &rTexturedCubeVBO = m_uiTexturedCubeVBO, &rTexturedCubeVAO = m_uiTexturedCubeVAO, &rTexturedCubeEBO = m_uiTexturedCubeEBO;
		glGenVertexArrays(1, &rTexturedCubeVAO);
		glGenBuffers(1, &rTexturedCubeVBO);
		glGenBuffers(1, &rTexturedCubeEBO);

		glBindVertexArray(rTexturedCubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::TexturedVertexData), Primitives::Cube::TexturedVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::TexturedIndexData), Primitives::Cube::TexturedIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// colored cube
	{
		GLuint &rColoredCubeVBO = m_uiColoredCubeVBO, &rColoredCubeVAO = m_uiColoredCubeVAO, &rColoredCubeEBO = m_uiColoredCubeEBO;
		glGenVertexArrays(1, &rColoredCubeVAO);
		glGenBuffers(1, &rColoredCubeVBO);
		glGenBuffers(1, &rColoredCubeEBO);

		glBindVertexArray(rColoredCubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rColoredCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::ColoredVertexData), Primitives::Cube::ColoredVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::ColoredIndexData), Primitives::Cube::ColoredIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	// textured plane
	{
		GLuint &rTexturedPlaneVBO = m_uiTexturedPlaneVBO, &rTexturedPlaneVAO = m_uiTexturedPlaneVAO, &rTexturedPlaneEBO = m_uiTexturedPlaneEBO;
		glGenVertexArrays(1, &rTexturedPlaneVAO);
		glGenBuffers(1, &rTexturedPlaneVBO);
		glGenBuffers(1, &rTexturedPlaneEBO);

		glBindVertexArray(rTexturedPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::TexturedVertexData), Primitives::Plane::TexturedVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::TexturedIndexData), Primitives::Plane::TexturedIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// colored plane
	{
		GLuint &rColoredPlaneVBO = m_uiColoredPlaneVBO, &rColoredPlaneVAO = m_uiColoredPlaneVAO, &rColoredPlaneEBO = m_uiColoredPlaneEBO;
		glGenVertexArrays(1, &rColoredPlaneVAO);
		glGenBuffers(1, &rColoredPlaneVBO);
		glGenBuffers(1, &rColoredPlaneEBO);

		glBindVertexArray(rColoredPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rColoredPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::ColoredVertexData), Primitives::Plane::ColoredVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::ColoredIndexData), Primitives::Plane::ColoredIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	// a textured sphere
	{
		// generate vertex data first.
		const int iNumberOfIterations = 2;
		SphereTrianglesGenerationResult tResult = GenerateSphereVertexData(Primitives::Sphere::SphereDefaultRadius, iNumberOfIterations);
		Primitives::Sphere::NumberOfTrianglesInSphere = tResult.m_uiNumberOfTriangles;

		GLuint &rTexturedSphereVBO = m_uiTexturedSphereVBO, &rTexturedSphereVAO = m_uiTexturedSphereVAO;// &rTexturedSphereEBO = m_uiTexturedSphereEBO;
		glGenVertexArrays(1, &rTexturedSphereVAO);
		glGenBuffers(1, &rTexturedSphereVBO);
		//glGenBuffers(1, &rTexturedSphereEBO);

		glBindVertexArray(rTexturedSphereVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedSphereVBO);
		glBufferData(GL_ARRAY_BUFFER, Primitives::Sphere::NumberOfTrianglesInSphere * sizeof(TriangularFace), tResult.m_pTriangleData, GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedSphereEBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::TexturedIndexData), Primitives::Plane::TexturedIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		delete[] tResult.m_pTriangleData;
	}
	
}

void Renderer::InitUniformBuffers()
{
	assert(m_tColorShader.IsInitialized() && m_tTextureShader.IsInitialized()); // need constructed shaders to link

	GLuint& rCameraProjectionUBO = m_uiCameraProjectionUBO;
	glGenBuffers(1, &rCameraProjectionUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, rCameraProjectionUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);	// dynamic draw because the camera matrix will change every frame
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// defining the range of the buffer, which is 2 mat4s
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, rCameraProjectionUBO, 0, 2 * sizeof(glm::mat4));
}

void Renderer::SetInitialOpenGLState()
{
	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glLineWidth(2.0f);
	glEnable(GL_LINE_SMOOTH);
}

void Renderer::FreeGPUResources()
{
	// Vertex Buffers and Vertey Arrays
	glDeleteVertexArrays(1, &m_uiTexturedCubeVAO);
	glDeleteBuffers(1, &m_uiTexturedCubeVBO);
	glDeleteBuffers(1, &m_uiTexturedCubeEBO);

	glDeleteVertexArrays(1, &m_uiColoredCubeVAO);
	glDeleteBuffers(1, &m_uiColoredCubeVBO);
	glDeleteBuffers(1, &m_uiColoredCubeEBO);

	glDeleteVertexArrays(1, &m_uiTexturedPlaneVAO);
	glDeleteBuffers(1, &m_uiTexturedPlaneVBO);
	glDeleteBuffers(1, &m_uiTexturedPlaneEBO);

	glDeleteVertexArrays(1, &m_uiColoredPlaneVAO);
	glDeleteBuffers(1, &m_uiColoredPlaneVBO);
	glDeleteBuffers(1, &m_uiColoredPlaneEBO);

	glDeleteVertexArrays(1, &m_uiTexturedSphereVAO);
	glDeleteBuffers(1, &m_uiTexturedSphereVBO);
	//glDeleteBuffers(1, &m_uiTexturedSphereEBO);

	// Uniform Buffers
	glDeleteBuffers(1, &m_uiCameraProjectionUBO);

	// Textures
	glDeleteTextures(1, &m_uiTexture1);
	glDeleteTextures(2, &m_uiTexture2);
}

void Renderer::RenderFrame(const Camera & rCamera, Window & rWindow, GUI& rGUI)
{
	glClearColor(m_vec4fClearColor.r, m_vec4fClearColor.g, m_vec4fClearColor.b, m_vec4fClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Render3DScene(rCamera, rWindow);
	rGUI.Render(rWindow);
}

void Renderer::Render3DScene(const Camera& rCamera, const Window& rWindow)
{
	// start by updating the uniform buffer containing the camera and projection matrices
	glBindBuffer(GL_UNIFORM_BUFFER, m_uiCameraProjectionUBO);
		// camera
		glm::mat4 mat4Camera = rCamera.GetViewMatrix();
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4Camera), glm::value_ptr(mat4Camera));
		// projection
		// NOTE: current world space transformations do not work with ortho matrix as defined below
		// glm::mat4 projection = glm::ortho(m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop, m_fNearPlane, m_fFarPlane);
		glm::mat4 mat4Projection = glm::perspective(glm::radians(rCamera.Zoom), (float)rWindow.m_iWindowWidth / (float)rWindow.m_iWindowHeight, m_fNearPlane, m_fFarPlane);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4Camera), sizeof(mat4Projection), glm::value_ptr(mat4Projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	

	// textured cube
	{
		glAssert();

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		m_tTextureShader.use();
		glAssert();
		m_tTextureShader.setInt("texture1", 0);
		m_tTextureShader.setInt("texture2", 1);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture2);

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(0.0f, -150.0f, 0.0f));
		m_tTextureShader.setMat4("world", world);

		glAssert();

		// render cube
		glBindVertexArray(m_uiTexturedCubeVAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::TexturedIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

		glAssert();
	}

	// colored line cube
	{
		glAssert();

		m_tColorShader.use();

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(0.0f, 150.0f, 0.0f));
		m_tColorShader.setMat4("world", world);

		// pass color
		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
		m_tColorShader.setVec4("color", vec4FullGreenColor);

		glAssert();

		// render the colored lined cube with GL_LINE_STRIP
		glBindVertexArray(m_uiColoredCubeVAO);
		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::ColoredIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

		glAssert();
	}

	// textured plane
	{
		glAssert();

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		m_tTextureShader.use();
		glAssert();
		m_tTextureShader.setInt("texture1", 0);
		m_tTextureShader.setInt("texture2", 1);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture2);

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(-150.0f, -100.0f, 0.0f));
		m_tTextureShader.setMat4("world", world);

		glAssert();

		// render cube
		glBindVertexArray(m_uiTexturedPlaneVAO);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::TexturedIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

		glAssert();
	}

	// colored line plane
	{
		glAssert();

		m_tColorShader.use();

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(150.0f, -100.0f, 0.0f));
		m_tColorShader.setMat4("world", world);

		// pass color
		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
		m_tColorShader.setVec4("color", vec4FullGreenColor);

		glAssert();

		// render the colored lined cube with GL_LINE_STRIP
		glBindVertexArray(m_uiColoredPlaneVAO);
		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Plane::ColoredIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

		glAssert();
	}

	// a textured sphere
	{
		glAssert();

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		m_tTextureShader.use();
		glAssert();
		m_tTextureShader.setInt("texture1", 0);
		m_tTextureShader.setInt("texture2", 1);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_uiTexture2);

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(-150.0f, 0.0f, 0.0f));
		m_tTextureShader.setMat4("world", world);

		glAssert();

		// render cube
		glBindVertexArray(m_uiTexturedSphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);

		glAssert();
	}

	// colored line sphere
	{
		glAssert();

		m_tColorShader.use();

		glAssert();

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		world = glm::translate(world, glm::vec3(150.0f, -100.0f, 150.0f));
		m_tColorShader.setMat4("world", world);

		// pass color
		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
		m_tColorShader.setVec4("color", vec4FullGreenColor);

		glAssert();

		// render the colored lined cube with GL_LINE_STRIP
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		glAssert();
	}
}