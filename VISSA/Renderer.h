#pragma once

#include "generalGL.h"

#include "Shader.h"

class Camera;
struct Window;
class GUI;
class Scene;

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
	Shader m_tColorShader, m_tTextureShader, m_tMaskedColorShader;
	// Buffer Handles
	GLuint m_uiCameraProjectionUBO;
	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiColoredCubeVBO, m_uiColoredCubeVAO, m_uiColoredCubeEBO;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_uiColoredPlaneVBO, m_uiColoredPlaneVAO, m_uiColoredPlaneEBO;
	GLuint m_uiTexturedSphereVBO, m_uiTexturedSphereVAO;// m_uiTexturedSphereEBO;
	GLuint m_uiGridPlaneVBO, m_uiGridPlaneVAO, m_uiGridPlaneEBO;
	// Texture Handles
	GLuint m_uiTexture1, m_uiTexture2, m_uiGridMaskTexture;
	// View space information
	float m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop;
	float m_fNearPlane, m_fFarPlane;
	glm::vec4 m_vec4fClearColor;

	// Init/Loads/Frees
	void InitRenderer();
	// Work
	void RenderScene(const Camera& rCamera, Window& rWindow, const Scene& rScene);
private:
	SphereTrianglesGenerationResult GenerateSphereVertexData(float fRadius, int SubdivisionIterations);
	void LoadShaders();
	void LoadTextures();
	GLuint LoadTextureFromFile(const char* sPath);
	void LoadPrimitivesToGPU();
	void InitUniformBuffers();
	void SetInitialOpenGLState();
	void Render3DScene(const Camera& rCamera, const Window& rWindow, const Scene& rScene);
	void Render3DSceneConstants(const Camera& rCamera, const Window& rWindow);	// scene components that are omnipresent, like a uniform grid or a cross hair
	void RenderRealObjects(const Camera& rCamera, const Window& rWindow, const Scene& rScene);
	void RenderRealObjectsOLD(const Camera& rCamera, const Window& rWindow, const Scene& rScene);
	void RenderDataStructureObjectsOLD(const Camera& rCamera, const Window& rWindow);
	void RenderDataStructureObjects(const Camera& rCamera, const Window& rWindow, const Scene& rScene);
	void FreeGPUResources();
};