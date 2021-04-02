#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "generalGL.h"
#include "Shader.h"
#include "Camera.h"
#include "Window.h"
#include "GUI.h"
#include "GeometricPrimitiveData.h"
#include "Visualization.h"

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
	// frame constant matrices
	m_mat4Camera(glm::mat4(1.0f)), // identity matrix for starters
	m_mat4PerspectiveProjection(glm::mat4(1.0f)), // identity matrix for starters
	m_mat4OrthographicProjection(glm::mat4(1.0f)), // identity matrix for starters
	// members defining the view frustums
	m_fOrthoLeft(0.0f),
	m_fOrthoRight(1280.0f),
	m_fOrthoBottom(0.0f),
	m_fOrthoTop(720.0f),
	m_fNearPlane(0.1f),
	m_fFarPlane(10000.0f),
	// clear color: a light grey
	m_vec4fClearColor(0.3f, 0.3f, 0.3f, 1.0f)
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

const glm::mat4 & Renderer::GetCameraMatrix() const
{
	return m_mat4Camera;
}

const glm::mat4 & Renderer::GetPerspectiveProjectionMatrix() const
{
	return m_mat4PerspectiveProjection;
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

	// A masked color shader
	Shader tMaskedColorShader("resources/shaders/MaskedColor.vs", "resources/shaders/MaskedColor.frag");
	m_tMaskedColorShader = tMaskedColorShader;
	assert(m_tMaskedColorShader.IsInitialized());

	// A crosshaird (hud component) shader
	Shader tCrossHairShader("resources/shaders/crosshair.vs", "resources/shaders/crosshair.frag");
	m_tCrosshairShader = tCrossHairShader;
	assert(m_tCrosshairShader.IsInitialized());
}

void Renderer::LoadTextures()
{
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	m_uiTexture1 = LoadTextureFromFile("resources/textures/cobblestone_floor_13_diff_1k.jpg");
	m_uiTexture2 = LoadTextureFromFile("resources/textures/cobblestone_floor_13_diff_1k.png");
	m_uiGridMaskTexture = LoadTextureFromFile("resources/textures/grid_mask_transparent.png");
	m_uiCrosshairTexture = LoadTextureFromFile("resources/textures/crosshair.png");
}

GLuint Renderer::LoadTextureFromFile(const char * sPath)
{
	unsigned int uiTextureID;
	glGenTextures(1, &uiTextureID);

	int iTextureWidth, iTextureHeight, iNumberChannels;
	unsigned char *data = stbi_load(sPath, &iTextureWidth, &iTextureHeight, &iNumberChannels, 0);
	if (data)
	{
		GLenum eFormat;
		if (iNumberChannels == 1)
			eFormat = GL_RED;
		else if (iNumberChannels == 3)
			eFormat = GL_RGB;
		else if (iNumberChannels == 4)
			eFormat = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, uiTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, eFormat, iTextureWidth, iTextureHeight, 0, eFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << sPath << std::endl;
		stbi_image_free(data);
	}

	return uiTextureID;
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
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::VertexData), Primitives::Cube::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::IndexData), Primitives::Cube::IndexData, GL_STATIC_DRAW);

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::SimpleVertexData), Primitives::Cube::SimpleVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::SimpleIndexData), Primitives::Cube::SimpleIndexData, GL_STATIC_DRAW);

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::VertexData), Primitives::Plane::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

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
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::SimpleVertexData), Primitives::Plane::SimpleVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::SimpleIndexData), Primitives::Plane::SimpleIndexData, GL_STATIC_DRAW);

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
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		//delete[] tResult.m_pTriangleData;
		Primitives::Sphere::VertexData = &tResult.m_pTriangleData->vertex1.vec3Position.x;//reinterpret_cast<GLfloat*>(tResult.m_pTriangleData);
	}
	

	// plane for uniform grid display
	{
		GLuint &rGridPlaneVBO = m_uiGridPlaneVBO, &rGridPlaneVAO = m_uiGridPlaneVAO, &rGridPlaneEBO = m_uiGridPlaneEBO;
		glGenVertexArrays(1, &rGridPlaneVAO);
		glGenBuffers(1, &rGridPlaneVBO);
		glGenBuffers(1, &rGridPlaneEBO);

		glBindVertexArray(rGridPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rGridPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Specials::GridPlane::VertexData), Primitives::Specials::GridPlane::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rGridPlaneEBO); // index data is equal to that of a normal plane
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

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

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	glDeleteTextures(1, &m_uiTexture2);
	glDeleteTextures(1, &m_uiGridMaskTexture);
}

void Renderer::UpdateFrameConstants(const Camera& rCamera, const Window& rWindow)
{
	// camera matrix
	m_mat4Camera = rCamera.GetViewMatrix();
}

void Renderer::UpdateProjectionMatrices(const Camera& rCamera, const Window& rWindow)
{
	// perspective projection matrix
	m_mat4PerspectiveProjection = glm::perspective(glm::radians(rCamera.Zoom), (float)rWindow.m_iWindowWidth / (float)rWindow.m_iWindowHeight, m_fNearPlane, m_fFarPlane);

	// orthographic projection matrix
	m_mat4OrthographicProjection = glm::ortho(m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop, m_fNearPlane, m_fFarPlane);
}

void Renderer::Render(const Camera & rCamera, Window & rWindow, const Visualization& rScene)
{
	if (rWindow.IsMinimized()) // hot fix to stop crashes when minimizing the window. needs proper handling in the future: https://www.glfw.org/docs/3.3/window_guide.html
		return;

	glClearColor(m_vec4fClearColor.r, m_vec4fClearColor.g, m_vec4fClearColor.b, m_vec4fClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UpdateFrameConstants(rCamera, rWindow);
	UpdateProjectionMatrices(rCamera, rWindow);
	RenderVisualization(rCamera, rWindow, rScene);
}

glm::vec3 Renderer::ConstructRayDirectionFromMousePosition(const Window& rWindow) const
{
	// from: https://antongerdelan.net/opengl/raycasting.html

	const Window::MousePositionInWindow tMousePosition = rWindow.GetCurrentMousePosition();

	// screen space (viewport coordinates)
	float x = (2.0f * tMousePosition.m_fXPosition) / static_cast<float>(rWindow.m_iWindowWidth) - 1.0f;
	float y = 1.0f - (2.0f * tMousePosition.m_fYPosition) / static_cast<float>(rWindow.m_iWindowHeight);
	float z = 1.0f;
	// normalised device space
	glm::vec3 ray_nds = glm::vec3(x, y, z);
	// clip space
	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
	// camera space
	glm::vec4 ray_eye = glm::inverse(m_mat4PerspectiveProjection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	// world space
	glm::vec3 ray_world = glm::vec3(glm::inverse(m_mat4Camera) * ray_eye);
	// don't forget to normalise the vector at some point
	ray_world = glm::normalize(ray_world);
	return ray_world;
}

void Renderer::RenderVisualization(const Camera& rCamera, const Window& rWindow, const Visualization& rScene)
{
	// start by updating the uniform buffer containing the camera and projection matrices
	glBindBuffer(GL_UNIFORM_BUFFER, m_uiCameraProjectionUBO);
		// camera
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_mat4Camera), glm::value_ptr(m_mat4Camera));
		// perspective projection
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m_mat4Camera), sizeof(m_mat4PerspectiveProjection), glm::value_ptr(m_mat4PerspectiveProjection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	
	RenderRealObjects(rCamera, rWindow, rScene);
	//RenderRealObjectsOLD(rCamera, rWindow, rVisualization);
	RenderDataStructureObjects(rCamera, rWindow, rScene);
	Render3DSceneConstants(rCamera, rWindow, rScene);
	RenderHUDComponents(rCamera, rWindow, rScene);
}

void Renderer::Render3DSceneConstants(const Camera & rCamera, const Window & rWindow, const Visualization& rScene)
{
	// uniform grid
	{
		glAssert();

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		Shader& rCurrentShader = m_tMaskedColorShader;
		rCurrentShader.use();
		glAssert();
		rCurrentShader.setInt("transparencyMask", 0);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiGridMaskTexture);

		glAssert();

		glDisable(GL_CULL_FACE);

		glBindVertexArray(m_uiGridPlaneVAO);

		if (rScene.m_bRenderGridXPlane)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 mat4WorldXPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldXPlane = glm::translate(mat4WorldXPlane, glm::vec3(rScene.m_vec3GridPositionsOnAxes.x, 0.0f, 0.0f));
			mat4WorldXPlane = glm::rotate(mat4WorldXPlane, glm::pi<float>() * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			rCurrentShader.setMat4("world", mat4WorldXPlane);

			// setting the color
			rCurrentShader.setVec4("color", rScene.m_vec4GridColorX);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}

		if (rScene.m_bRenderGridYPlane)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 mat4WorldYPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldYPlane = glm::translate(mat4WorldYPlane, glm::vec3(0.0f, rScene.m_vec3GridPositionsOnAxes.y, 0.0f));
			rCurrentShader.setMat4("world", mat4WorldYPlane);
			// no rotation needed since the default rendered plane is defined as lying flat on the "ground", facing upwards

			// setting the color			
			rCurrentShader.setVec4("color", rScene.m_vec4GridColorY);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}

		if (rScene.m_bRenderGridZPlane)
		{
			glm::mat4 mat4WorldZPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldZPlane = glm::translate(mat4WorldZPlane, glm::vec3(0.0f, 0.0f, -rScene.m_vec3GridPositionsOnAxes.z));
			mat4WorldZPlane = glm::rotate(mat4WorldZPlane, glm::pi<float>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
			rCurrentShader.setMat4("world", mat4WorldZPlane);

			// setting the color
			rCurrentShader.setVec4("color", rScene.m_vec4GridColorZ);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}	

		glEnable(GL_CULL_FACE);

		glAssert();
	}
}

void Renderer::RenderHUDComponents(const Camera & rCamera, const Window & rWindow, const Visualization & rVisualization)
{
	// Crosshair
	{
		glAssert();
		Shader& rCurrentShader = m_tCrosshairShader;
		rCurrentShader.use();
		glAssert();
		rCurrentShader.setInt("transparencyMask", 0);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiCrosshairTexture);

		glBindVertexArray(m_uiTexturedPlaneVAO);

		// world matrix
		glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
		glm::vec3 vec3CrosshairTranslationVector(static_cast<float>(rWindow.m_iWindowWidth) * 0.5f, static_cast<float>(rWindow.m_iWindowHeight) * 0.5f, -(m_fNearPlane + 0.0f)); // in the middle of the window, within the near plane of the view frustum
		mat4World = glm::translate(mat4World, vec3CrosshairTranslationVector);
		mat4World = glm::scale(mat4World, glm::vec3(rVisualization.m_fCrossHairScaling, rVisualization.m_fCrossHairScaling, 1.0f));
		mat4World = glm::rotate(mat4World, glm::pi<float>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)); // default plane/quad is defined as lying face up flat on the floor. this makes it "stand up" and face the camera
		rCurrentShader.setMat4("world", mat4World);

		// no camera matrix for hud components!

		// projection matrix
		rCurrentShader.setMat4("orthoProjection", m_mat4OrthographicProjection);

		// setting the color
		rCurrentShader.setVec4("color", rVisualization.m_vec4CrossHairColor);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
	}
}

void Renderer::RenderRealObjects(const Camera & rCamera, const Window & rWindow, const Visualization & rScene)
{
	glAssert();
	m_tTextureShader.use();
	glAssert();
	m_tTextureShader.setInt("texture1", 0);
	m_tTextureShader.setInt("texture2", 1);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_uiTexture2);

	for (const SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		const SceneObject::Transform& rCurrentTransform = rCurrentSceneObject.m_tTransform;

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
		// translation
		world = glm::translate(world, rCurrentTransform.m_vec3Position);
		// rotation
		world = glm::rotate(world, glm::radians(rCurrentTransform.m_tRotation.m_fAngle), rCurrentTransform.m_tRotation.m_vec3Axis);
		// scale
		world = glm::scale(world, rCurrentTransform.m_vec3Scale);
		m_tTextureShader.setMat4("world", world);

		glAssert();

		// render
		if (rCurrentSceneObject.m_eType == SceneObject::eType::CUBE) 
		{
			glBindVertexArray(m_uiTexturedCubeVAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}
		else if (rCurrentSceneObject.m_eType == SceneObject::eType::SPHERE)
		{
			glBindVertexArray(m_uiTexturedSphereVAO);
			glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
		}
		else
		{
			assert(!"disaster :)");
		}
	}
}

void Renderer::RenderRealObjectsOLD(const Camera & rCamera, const Window & rWindow, const Visualization& rScene)
{
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
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

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
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

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
}

void Renderer::RenderDataStructureObjectsOLD(const Camera & rCamera, const Window & rWindow)
{
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
		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

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
		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Plane::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

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
		glBindVertexArray(m_uiTexturedSphereVAO);
		glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		glAssert();
	}
}

void Renderer::RenderDataStructureObjects(const Camera & rCamera, const Window & rWindow, const Visualization& rVisualization)
{
	Shader& rCurrentShader = m_tColorShader;
	rCurrentShader.use();
	glAssert();

	glDisable(GL_CULL_FACE);
	
	// AABBs
	if (rVisualization.m_bRenderObjectAABBs)
	{
		// render colour yellow for AABBs
		glm::vec4 vec4AABBRenderColor(1.0f, 1.0f, 0.0f, 1.0f);
		rCurrentShader.setVec4("color", rVisualization.m_vec4AABBDefaultColor);

		for (const SceneObject& rCurrentSceneObject : rVisualization.m_vecObjects)
		{
			RenderAABBOfSceneObject(rCurrentSceneObject, rCurrentShader);
		}
	}

	// Bounding Spheres
	if (rVisualization.m_bRenderObjectBoundingSpheres)
	{
		rCurrentShader.setVec4("color", rVisualization.m_vec4BoundingSphereDefaultColor);

		for (const SceneObject& rCurrentSceneObject : rVisualization.m_vecObjects)
		{
			RenderBoundingSphereOfSceneObject(rCurrentSceneObject, rCurrentShader);
		}
	}

	if (rVisualization.GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::AABB) 
	{
		// rendering the AABBs of tree nodes in the BVH
		if (rVisualization.GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			int16_t iAlreadyRenderedConstructionSteps = 0;
			for (const CollisionDetection::TreeNodeForRendering& rCurrentRenderedBVHAABB : rVisualization.m_vecTreeAABBsForTopDownRendering)
			{
				bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRenderedBVHAABB.m_iTreeDepth <= rVisualization.m_iMaximumRenderedTreeDepth);
				bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < rVisualization.m_iNumberStepsRendered);

				bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
				if (bShallRender)
				{
					rCurrentShader.setVec4("color", rVisualization.m_vec4TopDownNodeRenderColor);
					RenderTreeNodeAABB(rCurrentRenderedBVHAABB, rCurrentShader);
				}

				iAlreadyRenderedConstructionSteps++;
			}
		}

		if (rVisualization.GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::BOTTOMUP)
		{
			int16_t iAlreadyRenderedConstructionSteps = 0;
			for (const CollisionDetection::TreeNodeForRendering& rCurrentRenderedBVHAABB : rVisualization.m_vecTreeAABBsForBottomUpRendering)
			{
				bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRenderedBVHAABB.m_iTreeDepth <= rVisualization.m_iMaximumRenderedTreeDepth);
				bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < rVisualization.m_iNumberStepsRendered);

				bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
				if (bShallRender)
				{
					rCurrentShader.setVec4("color", rVisualization.m_vec4BottomUpNodeRenderColor);
					RenderTreeNodeAABB(rCurrentRenderedBVHAABB, rCurrentShader);
				}

				iAlreadyRenderedConstructionSteps++;
			}
		}
	}
	else if (rVisualization.GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::BOUNDING_SPHERE)
	{
		if (rVisualization.GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			int16_t iAlreadyRenderedConstructionSteps = 0;
			for (const CollisionDetection::TreeNodeForRendering& rCurrentRenderedBVHBoundingSphere : rVisualization.m_vecTreeBoundingSpheresForTopDownRendering)
			{
				bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRenderedBVHBoundingSphere.m_iTreeDepth <= rVisualization.m_iMaximumRenderedTreeDepth);
				bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < rVisualization.m_iNumberStepsRendered);

				bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
				if (bShallRender)
				{
					rCurrentShader.setVec4("color", rVisualization.m_vec4TopDownNodeRenderColor);
					RenderTreeNodeBoundingsphere(rCurrentRenderedBVHBoundingSphere, rCurrentShader);
				}

				iAlreadyRenderedConstructionSteps++;
			}
		}
	}
	else
	{
		assert(!"disaster");
	}
	

	glEnable(GL_CULL_FACE);
}

void Renderer::RenderAABBOfSceneObject(const SceneObject & rSceneObject, Shader & rShader)
{
	// the AABBs
	const CollisionDetection::AABB& rRenderedAABB = rSceneObject.m_tWorldSpaceAABB;
	const CollisionDetection::AABB& rLocalSpaceAABBReference = rSceneObject.m_tLocalSpaceAABB;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedAABB.m_vec3Center);
	// scale
	world = glm::scale(world, rRenderedAABB.m_vec3Radius / rLocalSpaceAABBReference.m_vec3Radius);

	rShader.setMat4("world", world);

	glAssert();

	// render the object appropriately
	glBindVertexArray(m_uiColoredCubeVAO);
	glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glAssert();
}

void Renderer::RenderBoundingSphereOfSceneObject(const SceneObject & rSceneObject, Shader & rShader)
{
	const CollisionDetection::BoundingSphere& rRenderedBoundingSphere = rSceneObject.m_tWorldSpaceBoundingSphere;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedBoundingSphere.m_vec3Center);
	// scale 
	const float fScale = rRenderedBoundingSphere.m_fRadius / Primitives::Sphere::SphereDefaultRadius;
	world = glm::scale(world, glm::vec3(fScale, fScale, fScale));

	rShader.setMat4("world", world);

	glAssert();

	// render a sphere
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(m_uiTexturedSphereVAO);
	glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::RenderTreeNodeAABB(const CollisionDetection::TreeNodeForRendering & rTreeNodeAABB, Shader & rShader)
{
	// the AABB
	const CollisionDetection::AABB& rRenderedAABB = rTreeNodeAABB.m_pNodeToBeRendered->m_tAABBForNode;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedAABB.m_vec3Center);
	// scale
	const float fDetaultCubeHalfWidth = Primitives::Cube::DefaultCubeHalfWidth;
	world = glm::scale(world, rRenderedAABB.m_vec3Radius / glm::vec3(fDetaultCubeHalfWidth, fDetaultCubeHalfWidth, fDetaultCubeHalfWidth)); // scaling a "default" cube so it has the same extents as the current AABB

	rShader.setMat4("world", world);

	glAssert();

	// render the cube
	glBindVertexArray(m_uiColoredCubeVAO);
	glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glAssert();
}

void Renderer::RenderTreeNodeBoundingsphere(const CollisionDetection::TreeNodeForRendering & rTreeNodeAABB, Shader & rShader)
{
	// the bounding sphere
	const CollisionDetection::BoundingSphere& rRenderedBoundingSphere = rTreeNodeAABB.m_pNodeToBeRendered->m_tBoundingSphereForNode;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedBoundingSphere.m_vec3Center);
	// scale
	const float fDefaultSphereRadius = Primitives::Sphere::SphereDefaultRadius;
	const float fRenderedSphereRadius = rRenderedBoundingSphere.m_fRadius / fDefaultSphereRadius;
	world = glm::scale(world,  glm::vec3(fRenderedSphereRadius, fRenderedSphereRadius, fRenderedSphereRadius)); // scaling a "default" sphere so it has the same extents as the current AABB

	rShader.setMat4("world", world);

	glAssert();

	// render the sphere
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(m_uiTexturedSphereVAO);
	glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glAssert();
}