#pragma once

#include <vector>

#include "SceneObject.h"
#include "CollisionDetection.h"
#include "Shader.h"

class Engine;
struct Window;
class Renderer;

/*
	This class needs the most reworking to be practically usable. instead of primitives, actual models should be saved here.
	Also, it is common to have "structure of arrays" as opposed to "array of structs"
*/
class Visualization {
public:
	Visualization();
	~Visualization();

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

	std::vector<SceneObject> m_vecObjects;
private:
	
public:
	Window* m_p2DGraphWindow;
	CollisionDetection::BoundingVolumeHierarchy m_tTopDownBVH_AABB;
	CollisionDetection::BoundingVolumeHierarchy m_tBottomUpBVH_AABB;
	CollisionDetection::BoundingVolumeHierarchy m_tTopDownBVH_BoundingSphere;
	CollisionDetection::BoundingVolumeHierarchy m_tBottomUpBVH_BoundingSphere;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeAABBsForTopDownRendering; // TODO: move these into the BVH objects
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeAABBsForBottomUpRendering;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeBoundingSpheresForTopDownRendering;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeBoundingSpheresForBottomUpRendering;
	Shader m_tMaskedColorShader2D, m_tColoredLineShader2D;
	GLuint m_uiTexturedPlaneVBO, m_uiTexturedPlaneVAO, m_uiTexturedPlaneEBO;
	GLuint m_ui2DLineVBO, m_ui2DLineVAO;
	GLuint m_ui2DCircleTexture, m_ui2DOBJTexture;
	glm::vec4 m_vec4GridColorX;
	glm::vec4 m_vec4GridColorY;
	glm::vec4 m_vec4GridColorZ;
	glm::vec4 m_vec4AABBDefaultColor;
	glm::vec4 m_vec4BoundingSphereDefaultColor;
	glm::vec4 m_vec4TopDownNodeRenderColor;
	glm::vec4 m_vec4TopDownNodeRenderColor_Gradient;
	glm::vec4 m_vec4BottomUpNodeRenderColor;
	glm::vec4 m_vec4BottomUpNodeRenderColor_Gradient;
	glm::vec4 m_vec4CrossHairColor;
	glm::vec3 m_vec3GridPositionsOnAxes;
	float m_fCrossHairScaling;
	float m_pPlaybackSpeeds[5]; // in Steps per second
	size_t m_uiCurrentPlayBackSpeedIndex;
	int8_t m_iSimulationDirectionSign; // a value of 1 or -1, 
	float m_fAccumulatedTimeSinceLastUpdateStep;

	mutable float m_f2DGraphNodeSize;
	mutable float m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;
	int16_t m_iMaximumRenderedTreeDepth;
	int m_iNumberStepsRendered; // the number of Tree nodes that is rendered. They are ordered by which was one was contructed first.
	ePresentationMode m_ePresentationMode;
	

	bool m_bRenderObjectAABBs;
	bool m_bRenderObjectBoundingSpheres;
	bool m_bRenderGridXPlane;
	bool m_bRenderGridYPlane;
	bool m_bRenderGridZPlane;	
	bool m_bNodeDepthColorGrading;
	
	void Load();
	void LoadDefaultScene();
	void ReconstructAllTrees();
	void UpdateAfterObjectPropertiesChange();
	void Update(float fDeltaTime);
	void RenderAdditionalWindows(Renderer& rRenderer) const;	// TODO: const Renderer?

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

	void InitPlaybackSpeeds();
	void InitRenderColors();

	glm::vec4 InterpolateRenderColorForTreeNode(const glm::vec4& rColor1, const glm::vec4& rColor2, int16_t iDepthInTree, int16_t iDeepestDepthOfNodes) const;

	// 2D graph
	void DrawBVHTreeGraph(const CollisionDetection::BoundingVolumeHierarchy& rBVH) const;
	void RecursiveDrawTreeGraph(const CollisionDetection::BVHTreeNode* pCurrentNode, ScreenSpaceForGraphRendering tScreenSpaceForThisNode, glm::vec2 vec2PreviousDrawPosition) const;
	void DrawNodeAtPosition(glm::vec2 vec2ScreenSpacePosition) const;
	void DrawLineFromTo(glm::vec2 vec2From, glm::vec2 vec2To) const;
	void Draw2DObjectAtPosition(glm::vec2 vec2ScreenSpacePosition) const;
};