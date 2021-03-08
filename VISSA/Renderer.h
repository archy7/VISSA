#pragma once

#include "generalGL.h"

#include "Shader.h"

class Camera;
struct Window;
class GUI;

struct TriangularFace;

/**
	The singular renderer class for all rendering
*/
class Renderer {
public:
	// Rule of Five
	Renderer();
	Renderer(const Renderer& rOther) = delete;				// no copy constructor
	Renderer(Renderer&& rOther) = delete;					// no assignment operator
	Renderer& operator=(const Renderer& rOther) = delete;	// no move constructor
	Renderer& operator=(Renderer&& rOther) = delete;		// no move assignment
	~Renderer();

private:
	// Member types
	struct SphereTrianglesGenerationResult {
		GLuint m_uiNumberOfTriangles;
		TriangularFace* m_pTriangleData;
	};
public:
	// Members
	/////////////////////////////////////////
	//////////////OPENGL/////////////////////
	/////////////////////////////////////////
	// shader handles
	Shader m_tColorShader, m_tTextureShader;
	// Buffer Handles
	GLuint m_uiCameraProjectionUBO;
	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiColoredCubeVBO, m_uiColoredCubeVAO, m_uiColoredCubeEBO;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_uiColoredPlaneVBO, m_uiColoredPlaneVAO, m_uiColoredPlaneEBO;
	GLuint m_uiTexturedSphereVBO, m_uiTexturedSphereVAO;// m_uiTexturedSphereEBO;
	// Texture Handles
	GLuint m_uiTexture1, m_uiTexture2;
	// View space information
	float m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop;
	float m_fNearPlane, m_fFarPlane;
	glm::vec4 m_vec4fClearColor;

	// Init/Loads/Frees
	void InitRenderer();
	// Work
	void RenderFrame(const Camera& rCamera, Window& rWindow, GUI& rGUI);
private:
	SphereTrianglesGenerationResult GenerateSphereVertexData(float fRadius, int SubdivisionIterations);
	void LoadShaders();
	void LoadTextures();
	void LoadPrimitivesToGPU();
	void InitUniformBuffers();
	void SetInitialOpenGLState();
	void FreeGPUResources();
	void Render3DScene(const Camera& rCamera, const Window& rWindow);
};