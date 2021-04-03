#pragma once

#include <vector>

#include "SceneObject.h"
#include "CollisionDetection.h"

class Engine;

/*
	This class needs the most reworking to be practically usable. instead of primitives, actual models should be saved here.
	Also, it is common to have "structure of arrays" as opposed to "array of structs"
*/
class Visualization {
public:
	Visualization();

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
	SceneObject* m_pCurrentlyFocusedObject;
public:
	CollisionDetection::BoundingVolumeHierarchy m_tTopDownBVH_AABB;
	CollisionDetection::BoundingVolumeHierarchy m_tBottomUpBVH_AABB;
	CollisionDetection::BoundingVolumeHierarchy m_tTopDownBVH_BoundingSphere;
	CollisionDetection::BoundingVolumeHierarchy m_tBottomUpBVH_BoundingSphere;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeAABBsForTopDownRendering;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeAABBsForBottomUpRendering;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeBoundingSpheresForTopDownRendering;
	std::vector<CollisionDetection::TreeNodeForRendering> m_vecTreeBoundingSpheresForBottomUpRendering;
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

	int16_t m_iMaximumRenderedTreeDepth;
	int m_iNumberStepsRendered; // the number of Tree nodes that is rendered. They are ordered by which was one was contructed first.
	ePresentationMode m_ePresentationMode;
	

	bool m_bRenderObjectAABBs;
	bool m_bRenderObjectBoundingSpheres;
	bool m_bRenderGridXPlane;
	bool m_bRenderGridYPlane;
	bool m_bRenderGridZPlane;	
	bool m_bNodeDepthColorGrading;
	
	void LoadDefaultScene();
	void ReconstructAllTrees();
	void UpdateAfterObjectPropertiesChange();
	void Update(float fDeltaTime);

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
	void SetFocusedObject(SceneObject* pFocusedObject);
	SceneObject* GetCurrentlyFocusedObject();
	void DeleteCurrentlyFocusedObject();
	void AddNewSceneObject(SceneObject& rNewSceneObject);
	void ClearCurrentScene();

private:
	eBVHConstructionStrategy m_eConstructionStrategy;
	eBVHBoundingVolume m_eBVHBoundingVolume;

	void InitPlaybackSpeeds();
	void InitRenderColors();
};