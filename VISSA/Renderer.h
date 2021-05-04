#pragma once

#include "generalGL.h"

#include "Shader.h"
#include "Visualization.h"
#include "CollisionDetection.h"

class Camera;
struct Window;
class GUI;
class Visualization;
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

	struct TriangularFace {
		struct Vertex {
			glm::vec3 vec3Position;
			glm::vec3 vec3Normal;
			glm::vec2 vec2UVs;
		};

		Vertex vertex1;
		Vertex vertex2;
		Vertex vertex3;
	};

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
private:
	// matrices
	glm::mat4 m_mat4Camera;
	glm::mat4 m_mat4PerspectiveProjection;
	glm::mat4 m_mat4OrthographicProjection;
public:
	
	// Init/Loads/Frees
	void InitRenderer();
	// Work
	// during a frame and before rendering, this updates some objects that will be constant for the duration of a frame, i.e. camera and projection matrix
	void UpdateFrameConstants(const Camera& rCamera, const Window& rWindow);
	void UpdateProjectionMatrices(const Camera& rCamera, const Window& rWindow);
	void RenderIntoMainWindow();

	static glm::vec3 ConstructRayDirectionFromMousePosition(const Window& rWindow, const glm::mat4& rmat4PerspectiveProjection, const glm::mat4& rmat4Camera);
	static GLuint LoadTextureFromFile(const char* sPath);
	// Getters
	static SphereTrianglesGenerationResult GenerateSphereVertexData(float fRadius, int SubdivisionIterations);

#ifdef _DEBUG
	static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);
#endif // _DEBUG
private:
	glm::vec4 m_vec4fMainWindowClearColor;

	void SetInitialOpenGLState();
	
	//void RenderRealObjectsOLD(const Camera& rCamera, const Window& rWindow, const Visualization& rScene);
	//void RenderDataStructureObjectsOLD(const Camera& rCamera, const Window& rWindow);
	void FreeGPUResources();
};