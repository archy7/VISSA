#pragma once

#include <vector>

#include <glad/glad.h>

#include "SceneObject.h"
#include "CollisionDetection.h"
#include "Shader.h"
#include "GUI.h"
#include "Camera.h"

class Engine;
struct Window;
class Renderer;

/*
	todo: doc
*/
class Visualization {
public:
	Visualization(Window& rMainWindow);
	~Visualization();

private:
	struct TreeNodeForRendering {
		CollisionDetection::BVHTreeNode* m_pNodeToBeRendered;
		int16_t m_iDepthInTree = 0u;
		int16_t m_iRenderingOrder = 0u; // when stepping through the simulation, this determines in which order node bounding volumes are rendered.
	};

	struct BVHRenderingDataTuple {
		CollisionDetection::BoundingVolumeHierarchy m_tBVH;
		std::vector<TreeNodeForRendering> m_vecTreeNodeDataForRendering;
		void DeleteAllData(){
			m_tBVH.DeleteTree();
			m_vecTreeNodeDataForRendering.clear();
		}
	};
public:
	enum ePresentationMode {
		DISCRETE = 0,
		CONTINUOUS
	};

	enum eBVHConstructionStrategy {
		TOPDOWN = 0,
		BOTTOMUP,
		NUM_BVHCONSTRUCTIONSTRATEGIES
	};

	enum eBVHBoundingVolume {
		AABB = 0,
		BOUNDING_SPHERE,
		NUM_BVHBOUNDINGVOLUMES
	};

	std::vector<SceneObject> m_vecObjects; // todo: turn this into a scene class

public:
	
private:
	BVHRenderingDataTuple m_tTopDownAABBs;
	BVHRenderingDataTuple m_tBottomUpAABBs;
	BVHRenderingDataTuple m_tTopDownBoundingSpheres;
	BVHRenderingDataTuple m_tBottomUpBoundingSpheres;
	BVHRenderingDataTuple* m_pCurrentlyActiveConstructionStrategy;
public:
	//struct WindodContext3D {
	// the window
	Window& m_rMainWindow;	// a reference because the main window will always survive the visualization and because the vis 100% requires it
	// camera
	Camera m_tCamera;
	// Transformation Matrices
	mutable glm::mat4 m_mat4Camera;
	mutable glm::mat4 m_mat4PerspectiveProjection3DWindow;
	mutable glm::mat4 m_mat4OrthographicProjection3DWindow;
	// Uniform Buffers
	GLuint m_uiCameraProjectionUBO;
	// Shaders
	Shader m_tColorShader, m_tFlatTextureShader, m_tMaskedColorShader, m_tHUDComponentColorShader;
	// Vertex Buffer, Element Buffer and Vertex Array Object Handles
	GLuint m_uiTexturedCubeVBO, m_uiTexturedCubeVAO, m_uiTexturedCubeEBO;
	GLuint m_uiColoredCubeVBO, m_uiColoredCubeVAO, m_uiColoredCubeEBO;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_uiColoredPlaneVBO, m_uiColoredPlaneVAO, m_uiColoredPlaneEBO;
	GLuint m_uiTexturedSphereVBO, m_uiTexturedSphereVAO;// m_uiTexturedSphereEBO;
	GLuint m_uiGridPlaneVBO, m_uiGridPlaneVAO, m_uiGridPlaneEBO;
	// Textures
	GLuint m_uiObjectDiffuseTexture, m_uiGridMaskTexture, m_uiCrosshairTexture;
	// Colors
	glm::vec4 m_vec4fClearColor3DSceneWindow;
	glm::vec4 m_vec4GridColorX;
	glm::vec4 m_vec4GridColorY;
	glm::vec4 m_vec4GridColorZ;
	glm::vec4 m_vec4AABBColor;
	glm::vec4 m_vec4BoundingSphereColor;
	glm::vec4 m_vec4TopDownNodeRenderColor;
	glm::vec4 m_vec4TopDownNodeRenderColor_Gradient;
	glm::vec4 m_vec4BottomUpNodeRenderColor;
	glm::vec4 m_vec4BottomUpNodeRenderColor_Gradient;
	glm::vec4 m_vec4CrossHairColor;
	// other options
	glm::vec3 m_vec3GridPositionsOnAxes;
	float m_fCrossHairScaling;
	bool m_bRenderObjectAABBs;
	bool m_bRenderObjectBoundingSpheres;
	bool m_bRenderGridXPlane;
	bool m_bRenderGridYPlane;
	bool m_bRenderGridZPlane;
	bool m_bNodeDepthColorGrading;
	//} m_tWindowContext3D;
	
	//struct WindowContext2D {
	// the window
	Window* m_p2DGraphWindow;	// todo: maybe this should be a reference instead of a pointer too since the windows lifetime is tied to the vis
	glm::mat4 m_mat4OrthographicProjection2DWindow;
	// Shaders
	Shader m_tMaskedColorShader2D, m_tColoredLineShader2D;
	// Vertex Buffer, Element Buffer and Vertex Array Object Handles
	GLuint m_uiTextured2DPlaneVBO, m_uiTextured2DPlaneVAO, m_uiTextured2DPlaneEBO;
	GLuint m_ui2DLineVBO, m_ui2DLineVAO;
	// Texture Handles
	GLuint m_ui2DCircleTexture, m_ui2DOBJTexture;
	// Colors
	glm::vec4 m_vec4fClearColor2DGraphWindow;
	// other options
	mutable float m_f2DGraphNodeSize;
	mutable float m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;
	//} m_tWindowContext2D;
	
	// GUI members
	SceneObject* m_pCurrentlyFocusedObject;
	ImVec2 m_vec2ObjectPropertiesWindowPosition;
	bool m_bGUICaptureMouse;
	bool m_bShowSimulationOptions;
	bool m_bShowObjectCreationWindow;
	bool m_bShowObjectPropertiesWindow;
	bool m_bShowHelpWindow;
	bool m_bObjectPropertiesPendingChanges;

	// members not tied to a specific window	
	float m_pPlaybackSpeeds[5]; // in Steps per second
	float m_fRenderDistance;
	size_t m_uiCurrentPlayBackSpeedIndex;
	int8_t m_iSimulationDirectionSign; // a value of 1 or -1, 
	float m_fAccumulatedTimeSinceLastUpdateStep;
	int16_t m_iMaximumRenderedTreeDepth;
	int m_iNumberStepsRendered; // the number of Tree nodes that is rendered. They are ordered by which was one was contructed first.
	ePresentationMode m_ePresentationMode;
	float m_fDeltaTime;	// Updated in Update(...); 
	

	void Load();
	void LoadDefaultScene();
	void Render();
	void ReconstructAllTrees();
	void UpdateAfterObjectPropertiesChange();
	void Update(float fDeltaTime);
	

	// input
	void ProcessMouseMovement(GLFWwindow* pWindow, double dXPosition, double dYPosition);
	void ProcessMouseClick(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers);
	void ProcessKeyboardInput();

	// simulation controls
	void ResetSimulation();
	void PlaySimulation();
	void PauseSimulation();
	void IncreaseSimulationSpeed();
	void DecreaseSimulationSpeed();
	float GetCurrentSimulationSpeed() const;
	void InvertSimulationProgression();			// instead of advancing "forward", the simulation now rolls "back", and vice versa
	void MoveToNextSimulationStep();

	

	// options, parameters and manipulation
	eBVHConstructionStrategy GetCurrenBVHConstructionStrategy() const;
	void SetNewBVHConstructionStrategy(eBVHConstructionStrategy eNewStrategy);
	eBVHBoundingVolume GetCurrentBVHBoundingVolume() const;
	void SetNewBVHBoundingVolume(eBVHBoundingVolume eNewBoundingVolume);
	
	void DeleteGivenObject(SceneObject* pToBeDeletedObject);
	void AddNewSceneObject(SceneObject& rNewSceneObject);
	void ClearCurrentScene();

private:

	struct ScreenSpaceForGraphRendering {
		float m_fWidthStart;
		float m_fWidthEnd;
		float m_fHeightStart;
		float m_fHeightEnd;
	};

	eBVHConstructionStrategy m_eConstructionStrategy;
	eBVHBoundingVolume m_eBVHBoundingVolume;

	


	/*
		recursive function that constructs a top down AABB tree
	*/
	void RecursiveTopDownTree_AABB(CollisionDetection::BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	CollisionDetection::BVHTreeNode* BottomUpTree_AABB(SceneObject* pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization);
	/*
		recursive function that constructs a top down Bounding Sphere tree
	*/
	void RecursiveTopDownTree_BoundingSphere(CollisionDetection::BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	CollisionDetection::BVHTreeNode* BottomUpTree_BoundingSphere(SceneObject* pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization);
	/*
		TODO: DOC
	*/
	void TraverseTreeForDataForTopDownRendering_AABB(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple& rBVHRenderDataTuple, int16_t iDepthInTree);
	/*
		TODO: DOC
	*/
	void TraverseTreeForDataForBottomUpRendering_AABB(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple& rBVHRenderDataTuple, int16_t iDepthInTree);
	/*
		TODO: DOC
	*/
	void TraverseTreeForDataForTopDownRendering_BoundingSphere(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple& rBVHRenderDataTuple, int16_t iDepthInTree);
	/*
		TODO: DOC
	*/
	void TraverseTreeForDataForBottomUpRendering_BoundingSphere(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple& rBVHRenderDataTuple, int16_t iDepthInTree);

	// loading and setup
	void InitPlaybackSpeeds();
	void InitRenderColors();
	void LoadTextures();
	void LoadShaders();
	void LoadPrimitivesToGPU();
	void InitUniformBuffers();
	void SetInitialRenderStates();

	// rendering
	void Render2DGraph() const;
	void UpdateFrameConstants();
	void UpdateProjectionMatrices();
	void Render3DVisualization() const;
	void RenderVisualizationGUI();
	void RenderRealObjects() const;
	void RenderHUDComponents() const;
	void RenderDataStructureObjects() const;
	void Render3DSceneConstants() const;
	void RenderTreeNodeAABB(const TreeNodeForRendering& rTreeNodeAABB, const Shader& rShader) const;
	void RenderTreeNodeBoundingsphere(const TreeNodeForRendering& rTreeNodeAABB, const Shader& rShader) const;
	void RenderAABBOfSceneObject(const SceneObject& rSceneObject, const Shader& rShader) const;
	void RenderBoundingSphereOfSceneObject(const SceneObject& rSceneObject, const Shader& rShader) const;
	void FreeGPUResources();
	glm::vec4 InterpolateRenderColorForTreeNode(const glm::vec4& rColor1, const glm::vec4& rColor2, int16_t iDepthInTree, int16_t iDeepestDepthOfNodes) const;

	// GUI
	void ShowObjectPropertiesWindow(bool bShowIt);
	void ToggleHelpWindow();
	void RenderSimControlPanel();
	void RenderSimOptions();
	void RenderObjectPropertiesWindow();
	void RenderObjectCreationWindow();
	void RenderHelpWindow();
	void CancelObjectPropertiesChanges();
	bool IsMouseCaptured() const;
	void SetCaptureMouse(bool bIsCapturedNow);
	void SetFocusedObject(SceneObject* pFocusedObject);
	void SetObjectPropertiesWindowPosition(float fXPosition, float fYPosition);

	// input
	void CrosshairClick();
	void CursorClick();

	// BVHs
	BVHRenderingDataTuple ConstructTopDownAABBBVHandRenderDataForScene(Visualization& rScene);
	BVHRenderingDataTuple ConstructBottomUpAABBBVHandRenderDataForScene(Visualization& rScene);
	BVHRenderingDataTuple ConstructTopDownBoundingSphereBVHandRenderDataForScene(Visualization& rScene);
	BVHRenderingDataTuple ConstructBottomUpBoundingSphereBVHandRenderDataForScene(Visualization& rScene);

	// 2D graph
	void ConstructBVHTreeGraph(const BVHRenderingDataTuple& rBVHRenderDataTuple) const;
	void RecursiveDrawTreeGraph(const CollisionDetection::BVHTreeNode* pCurrentNode, ScreenSpaceForGraphRendering tScreenSpaceForThisNode, glm::vec2 vec2PreviousDrawPosition) const;
	void DrawNodeAtPosition(glm::vec2 vec2ScreenSpacePosition) const;
	void DrawLineFromTo(glm::vec2 vec2From, glm::vec2 vec2To) const;
	void Draw2DObjectAtPosition(glm::vec2 vec2ScreenSpacePosition) const;
};