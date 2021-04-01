#pragma once

#include "generalGL.h"

#include "Shader.h"
#include "Visualization.h"
#include "CollisionDetection.h"

class Camera;
struct Window;
class GUI;
class Visualization;

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

	// Member types
private:
	struct SphereTrianglesGenerationResult {
		GLuint m_uiNumberOfTriangles;
		TriangularFace* m_pTriangleData;
	};
public:
	// Members
	/////////////////////////////////////////
	//////////////OPENGL/////////////////////
	/////////////////////////////////////////
private:
	// matrices
	glm::mat4 m_mat4Camera;
	glm::mat4 m_mat4PerspectiveProjection;
	glm::mat4 m_mat4OrthographicProjection;
public:
	// shader handles
	Shader m_tColorShader, m_tTextureShader, m_tMaskedColorShader, m_tCrosshairShader;
	// Buffer Handles
	GLuint m_uiCameraProjectionUBO;
	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiColoredCubeVBO, m_uiColoredCubeVAO, m_uiColoredCubeEBO;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_uiColoredPlaneVBO, m_uiColoredPlaneVAO, m_uiColoredPlaneEBO;
	GLuint m_uiTexturedSphereVBO, m_uiTexturedSphereVAO;// m_uiTexturedSphereEBO;
	GLuint m_uiGridPlaneVBO, m_uiGridPlaneVAO, m_uiGridPlaneEBO;
	// Texture Handles
	GLuint m_uiTexture1, m_uiTexture2, m_uiGridMaskTexture, m_uiCrosshairTexture;
	// View space information
	float m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop;
	float m_fNearPlane, m_fFarPlane;
	glm::vec4 m_vec4fClearColor;

	// Init/Loads/Frees
	void InitRenderer();
	// Work
	// during a frame and before rendering, this updates some objects that will be constant for the duration of a frame, i.e. camera and projection matrix
	void UpdateFrameConstants(const Camera& rCamera, const Window& rWindow);
	void UpdateProjectionMatrices(const Camera& rCamera, const Window& rWindow);
	void Render(const Camera& rCamera, Window& rWindow, const Visualization& rScene);

	glm::vec3 ConstructRayDirectionFromMousePosition(const Window& rWindow) const;

	// Getters
	const glm::mat4& GetCameraMatrix() const;
	const glm::mat4& GetPerspectiveProjectionMatrix() const;
private:
	SphereTrianglesGenerationResult GenerateSphereVertexData(float fRadius, int SubdivisionIterations);
	void LoadShaders();
	void LoadTextures();
	GLuint LoadTextureFromFile(const char* sPath);
	void LoadPrimitivesToGPU();
	void InitUniformBuffers();
	void SetInitialOpenGLState();
	void RenderVisualization(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	void Render3DSceneConstants(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);	// scene components that are omnipresent, like a uniform grid or a cross hair
	void RenderHUDComponents(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	void RenderRealObjects(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	void RenderRealObjectsOLD(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	void RenderDataStructureObjectsOLD(const Camera& rCamera, const Window& rWindow);
	void RenderDataStructureObjects(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	void RenderAABBOfSceneObject(const SceneObject& rSceneObject, Shader& rShader);
	void RenderBoundingSphereOfSceneObject(const SceneObject& rSceneObject, Shader& rShader);
	void RenderTreeNodeAABB(const CollisionDetection::TreeNodeForRendering& rTreeNodeAABB, Shader& rShader);
	void FreeGPUResources();
};