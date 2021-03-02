#pragma once

#include "generalGL.h"

#include <vector>

#include "Shader.h"

class Camera;
struct Window;

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


	// Members
	Shader m_tColorShader, m_tTextureShader;

	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiTexture1, m_uiTexture2;

	float m_fNearPlane, m_fFarPlane;

	// Init/Loads/Frees
	void InitRenderer();
	void LoadShaders();
	void LoadTextures();
	void LoadPrimitivesToGPU();
	void SetInitialOpenGLState();
	void FreeGPUResources();

	// Work
	void RenderFrame(const Camera& rCamera, const Window& rWindow);
};