#include "Renderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

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

Renderer::Renderer() :
	// frame constant matrices
	m_mat4Camera(glm::mat4(1.0f)), // identity matrix for starters
	m_mat4PerspectiveProjection(glm::mat4(1.0f)), // identity matrix for starters
	m_mat4OrthographicProjection(glm::mat4(1.0f)), // identity matrix for starters
	m_vec4fMainWindowClearColor(0.9f, 0.9f, 0.9f, 1.0f)
{
	
}

Renderer::~Renderer()
{
	FreeGPUResources();
}

void Renderer::InitRenderer()
{
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

void Renderer::SetInitialOpenGLState()
{
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis. This is here for now until I find a better place to put it
}

void Renderer::RenderIntoMainWindow()
{
	glAssert();

	glClearColor(m_vec4fMainWindowClearColor.r, m_vec4fMainWindowClearColor.g, m_vec4fMainWindowClearColor.b, m_vec4fMainWindowClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	glAssert();
}

void Renderer::FreeGPUResources()
{
	
}

void Renderer::UpdateFrameConstants(const Camera& rCamera, const Window& rWindow)
{
	// camera matrix
	//m_mat4Camera = rCamera.GetViewMatrix();
}

void Renderer::UpdateProjectionMatrices(const Camera& rCamera, const Window& rWindow)
{
	// perspective projection matrix
	//m_mat4PerspectiveProjection = glm::perspective(glm::radians(rCamera.Zoom), (float)rWindow.m_iWindowWidth / (float)rWindow.m_iWindowHeight, m_fNearPlane, m_fFarPlane);

	// orthographic projection matrix
	//m_mat4OrthographicProjection = glm::ortho(0.0f, static_cast<float>(rWindow.m_iWindowWidth), 0.0f, static_cast<float>(rWindow.m_iWindowHeight), m_fNearPlane, m_fFarPlane);
}

glm::vec3 Renderer::ConstructRayDirectionFromMousePosition(const Window& rWindow, const glm::mat4& rmat4PerspectiveProjection, const glm::mat4& rmat4Camera)
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
	glm::vec4 ray_eye = glm::inverse(rmat4PerspectiveProjection) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	// world space
	glm::vec3 ray_world = glm::vec3(glm::inverse(rmat4Camera) * ray_eye);
	// don't forget to normalise the vector at some point
	ray_world = glm::normalize(ray_world);
	return ray_world;
}

//void Renderer::RenderRealObjectsOLD(const Camera & rCamera, const Window & rWindow, const Visualization& rScene)
//{
//	// textured cube
//	{
//		glAssert();
//
//		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
//		// -------------------------------------------------------------------------------------------
//		m_tFlatTextureShader.use();
//		glAssert();
//		m_tFlatTextureShader.setInt("texture1", 0);
//		m_tFlatTextureShader.setInt("texture2", 1);
//
//		glAssert();
//
//		// bind textures on corresponding texture units
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, m_uiObjectDiffuseTexture);;
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(0.0f, -150.0f, 0.0f));
//		m_tFlatTextureShader.setMat4("world", world);
//
//		glAssert();
//
//		// render cube
//		glBindVertexArray(m_uiTexturedCubeVAO);
//		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
//
//		glAssert();
//	}
//
//	// textured plane
//	{
//		glAssert();
//
//		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
//		// -------------------------------------------------------------------------------------------
//		m_tFlatTextureShader.use();
//		glAssert();
//		m_tFlatTextureShader.setInt("texture1", 0);
//		m_tFlatTextureShader.setInt("texture2", 1);
//
//		glAssert();
//
//		// bind textures on corresponding texture units
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, m_uiObjectDiffuseTexture);
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(-150.0f, -100.0f, 0.0f));
//		m_tFlatTextureShader.setMat4("world", world);
//
//		glAssert();
//
//		// render cube
//		glBindVertexArray(m_uiTexturedPlaneVAO);
//		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
//
//		glAssert();
//	}
//
//	// a textured sphere
//	{
//		glAssert();
//
//		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
//		// -------------------------------------------------------------------------------------------
//		m_tFlatTextureShader.use();
//		glAssert();
//		m_tFlatTextureShader.setInt("texture1", 0);
//		m_tFlatTextureShader.setInt("texture2", 1);
//
//		glAssert();
//
//		// bind textures on corresponding texture units
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, m_uiObjectDiffuseTexture);
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(-150.0f, 0.0f, 0.0f));
//		m_tFlatTextureShader.setMat4("world", world);
//
//		glAssert();
//
//		// render cube
//		glBindVertexArray(m_uiTexturedSphereVAO);
//		glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
//
//		glAssert();
//	}
//}
//
//void Renderer::RenderDataStructureObjectsOLD(const Camera & rCamera, const Window & rWindow)
//{
//	// colored line cube
//	{
//		glAssert();
//
//		m_tColorShader.use();
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(0.0f, 150.0f, 0.0f));
//		m_tColorShader.setMat4("world", world);
//
//		// pass color
//		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
//		m_tColorShader.setVec4("color", vec4FullGreenColor);
//
//		glAssert();
//
//		// render the colored lined cube with GL_LINE_STRIP
//		glBindVertexArray(m_uiColoredCubeVAO);
//		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
//
//		glAssert();
//	}
//
//	// colored line plane
//	{
//		glAssert();
//
//		m_tColorShader.use();
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(150.0f, -100.0f, 0.0f));
//		m_tColorShader.setMat4("world", world);
//
//		// pass color
//		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
//		m_tColorShader.setVec4("color", vec4FullGreenColor);
//
//		glAssert();
//
//		// render the colored lined cube with GL_LINE_STRIP
//		glBindVertexArray(m_uiColoredPlaneVAO);
//		glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Plane::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
//
//		glAssert();
//	}
//
//	// colored line sphere
//	{
//		glAssert();
//
//		m_tColorShader.use();
//
//		glAssert();
//
//		// calculate the model matrix for each object and pass it to shader before drawing
//		glm::mat4 world = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
//		world = glm::translate(world, glm::vec3(150.0f, -100.0f, 150.0f));
//		m_tColorShader.setMat4("world", world);
//
//		// pass color
//		glm::vec4 vec4FullGreenColor(0.0f, 1.0f, 0.0f, 1.0f);
//		m_tColorShader.setVec4("color", vec4FullGreenColor);
//
//		glAssert();
//
//		// render the colored lined cube with GL_LINE_STRIP
//		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//		glBindVertexArray(m_uiTexturedSphereVAO);
//		glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
//		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//
//
//		glAssert();
//	}
//}

#ifdef _DEBUG
void Renderer::glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char * message, const void * userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif // _DEBUG