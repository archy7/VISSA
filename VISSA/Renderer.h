#pragma once

#include "generalGL.h"

#include <vector>

#include "Shader.h"

class Camera;
struct Window;

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
	Shader m_tColorShader, m_tTextureShader;

	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiColoredCubeVBO, m_uiColoredCubeVAO, m_uiColoredCubeEBO;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_uiColoredPlaneVBO, m_uiColoredPlaneVAO, m_uiColoredPlaneEBO;
	GLuint m_uiTexturedSphereVBO, m_uiTexturedSphereVAO;// m_uiTexturedSphereEBO;

	GLuint m_uiTexture1, m_uiTexture2;

	float m_fNearPlane, m_fFarPlane;

	// Init/Loads/Frees
	void InitRenderer();
private:
	SphereTrianglesGenerationResult GenerateSphereVertexData(float fRadius, int SubdivisionIterations);
	void LoadShaders();
	void LoadTextures();
	void LoadPrimitivesToGPU();
	void SetInitialOpenGLState();
	void FreeGPUResources();
public:
	

	// Work
	void RenderFrame(const Camera& rCamera, Window& rWindow);
	void Render3DScene(const Camera& rCamera, const Window& rWindow);
	void RenderGUI(Window& rWindow);
};