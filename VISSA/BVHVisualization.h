#pragma once

#include "Visualization.h"
#include "Scene.h"

#include <vector>

class BVHVisualization final : public Visualization {
public:
	BVHVisualization() = delete;			// pointer to main window is right now 100% required -> no default ctor
	BVHVisualization(Window* pMainWindow);	// pointer to main window is right now 100% required
	BVHVisualization(BVHVisualization& rOther) = delete;			// class is not meant to be copy constructed
	BVHVisualization& operator= (BVHVisualization other) = delete;	// same goes for assignment
	~BVHVisualization();
private:
	/*
		Data Types and enums
	*/
	struct TreeNodeForRendering {
		CollisionDetection::BVHTreeNode* m_pNodeToBeRendered;
		glm::vec2 m_vec2_2DNodeDrawPosition;
		glm::vec2 m_vec2_2DLineToParentOrigin;
		glm::vec2 m_vec2_2DLineToParentTarget;
		int16_t m_iDepthInTree = 0u;
		int16_t m_iRenderingOrder = 0u; // when stepping through the simulation, this determines in which order node bounding volumes are rendered.
	};


	struct BVHRenderingDataTuple {
		CollisionDetection::BoundingVolumeHierarchy m_tBVH;
		std::vector<TreeNodeForRendering> m_vecTreeNodeDataForRendering;
		std::vector<TreeNodeForRendering> m_vecTreeLeafDataForRendering; // currently only used for rendering in the graph window
		void DeleteAllData() {
			m_tBVH.DeleteTree();
			m_vecTreeNodeDataForRendering.clear();
			m_vecTreeLeafDataForRendering.clear();
		}
	};

	struct ScreenSpaceForGraphRendering {
		float m_fWidthStart;
		float m_fWidthEnd;
		float m_fHeightStart;
		float m_fHeightEnd;
	};

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

private:
	/*
		Members 
	*/
	Scene m_tScene;

	BVHRenderingDataTuple m_tTopDownAABBs;
	BVHRenderingDataTuple m_tBottomUpAABBs;
	BVHRenderingDataTuple m_tTopDownBoundingSpheres;
	BVHRenderingDataTuple m_tBottomUpBoundingSpheres;
	BVHRenderingDataTuple* m_pCurrentlyActiveConstructionStrategy;	// todo: update the GUI to refer to this, also use it for all rendering purposes

	/*
		Members related to the 3D Window
	*/
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

	/*
		Members related to the 2D graph window
	*/
	// the window
	Window* m_p2DGraphWindow;
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
	eBVHConstructionStrategy m_eConstructionStrategy;
	eBVHBoundingVolume m_eBVHBoundingVolume;
	
public:
	/*
		Member Functions
	*/
	// interface required by the engine
	void Load() override;
	void Render() override;
	void Update(float fDeltaTime) override;
	// callbacks
	virtual void MouseMoveCallback(GLFWwindow* pWindow, double dXPosition, double dYPosition) override;
	virtual void MouseClickCallback(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers) override;
	virtual void WindowResizeCallBack(GLFWwindow* pWindow, int iNewWidth, int iNewHeight) override;
	virtual void ProcessKeyboardInput() override;
private:
	void LoadDefaultScene(Scene& rSceneToLoadInto);	// makeshift implementation of loading a scene
	void ReconstructAllTrees();
	void UpdateAfterObjectPropertiesChange();

	// simulation controls
	void ResetSimulation();
	void PlaySimulation();
	void PauseSimulation();
	void IncreaseSimulationSpeed();
	void DecreaseSimulationSpeed();
	float GetCurrentSimulationSpeed() const;
	void InvertSimulationProgression();			// instead of advancing "forward", the simulation now rolls "back", and vice versa
	void MoveToPreviousSimulationStep();
	void MoveToNextSimulationStep();
	void AdvanceSimulationInCurrentDirection();

	// options and  parameters
	eBVHConstructionStrategy GetCurrenBVHConstructionStrategy() const;
	void SetNewBVHConstructionStrategy(eBVHConstructionStrategy eNewStrategy);
	eBVHBoundingVolume GetCurrentBVHBoundingVolume() const;
	void SetNewBVHBoundingVolume(eBVHBoundingVolume eNewBoundingVolume);

	// scene manipulation
	void DeleteGivenObject(SceneObject* pToBeDeletedObject);
	void AddNewSceneObject(SceneObject& rNewSceneObject);
	void ClearCurrentScene();

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
	void Render3DVisualization();
	void RenderVisualizationGUI();
	void RenderRealObjects() const;
	void RenderHUDComponents() const;
	void RenderDataStructureObjects() const;
	void Render3DSceneConstants() const;
	void RenderTreeNodeAABB(const TreeNodeForRendering& rTreeNodeAABB, const Shader& rShader) const;			// todo: reconsider if that shader parameter is really needed
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
	bool IsMouseCapturedByGUI() const;
	bool IsCameraModeActive() const;	// convenience function for better clarity. Camera mode is active when mouse is captured.
	void SetGUICaptureMouse(bool bIsCapturedNow);
	void SetFocusedObject(SceneObject* pFocusedObject);
	void SetObjectPropertiesWindowPosition(float fXPosition, float fYPosition);

	// input
	void CrosshairClick();
	void CursorClick();

	// BVHs
	BVHRenderingDataTuple ConstructTopDownAABBBVHandRenderDataForScene(Scene& rScene);
	BVHRenderingDataTuple ConstructBottomUpAABBBVHandRenderDataForScene(Scene& rScene);
	BVHRenderingDataTuple ConstructTopDownBoundingSphereBVHandRenderDataForScene(Scene& rScene);
	BVHRenderingDataTuple ConstructBottomUpBoundingSphereBVHandRenderDataForScene(Scene& rScene);

	// 2D graph
	void ConstructBVHTreeGraphRenderData(BVHRenderingDataTuple& rBVHRenderDataTuple);
	void RecursiveConstructTreeGraphRenderData(const CollisionDetection::BVHTreeNode* pCurrentNode, BVHRenderingDataTuple& rBVHRenderDataTuple, ScreenSpaceForGraphRendering tScreenSpaceForThisNode, glm::vec2 vec2PreviousDrawPosition);
	void DrawNodeAtPosition(glm::vec2 vec2ScreenSpacePosition, const glm::vec4& rvec4DrawColor) const;
	void Draw2DObjectAtPosition(glm::vec2 vec2ScreenSpacePosition, const glm::vec4& rvec4DrawColor) const;
	void DrawLineFromTo(glm::vec2 vec2From, glm::vec2 vec2To) const;

	/*
		recursive function that constructs a top down AABB tree
	*/
	void RecursiveTopDownTree_AABB(CollisionDetection::BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	CollisionDetection::BVHTreeNode* BottomUpTree_AABB(SceneObject* pSceneObjects, size_t uiNumSceneObjects, BVHRenderingDataTuple& rBVHRenderDataTuple);
	/*
		recursive function that constructs a top down Bounding Sphere tree
	*/
	void RecursiveTopDownTree_BoundingSphere(CollisionDetection::BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	CollisionDetection::BVHTreeNode* BottomUpTree_BoundingSphere(SceneObject* pSceneObjects, size_t uiNumSceneObjects, BVHRenderingDataTuple& rBVHRenderDataTuple);
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
	/*
		TODO: DOC
	*/
	TreeNodeForRendering* FindRenderDataOfNode(const CollisionDetection::BVHTreeNode* pNode, std::vector< TreeNodeForRendering>& rvecRenderData) const;

};