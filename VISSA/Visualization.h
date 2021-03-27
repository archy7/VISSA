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
		SIZE
	};

	std::vector<SceneObject> m_vecObjects;
	std::vector<CollisionDetection::TreeNodeAABBForRendering> m_vecTreeAABBsForTopDownRendering;
	std::vector<CollisionDetection::TreeNodeAABBForRendering> m_vecTreeAABBsForBottomUpRendering;
	float m_pPlaybackSpeeds[5]; // in Steps per second
	size_t m_uiCurrentPlayBackSpeedIndex;
	int8_t m_iSimulationDirectionSign; // a value of 1 or -1, 
	float m_fAccumulatedTimeSinceLastUpdateStep;

	int16_t m_iMaximumRenderedTreeDepth;
	int m_iNumberStepsRendered; // the number of Tree node AABBs that is rendered. They are ordered by which was one was contructed first.
	ePresentationMode m_ePresentationMode;
	

	bool m_bRenderObjectAABBs;
	bool m_bRenderObjectBoundingSpheres;
	bool m_bRenderGridXPlane;
	bool m_bRenderGridYPlane;
	bool m_bRenderGridZPlane;
	
	
	void Load();
	void Update(float fDeltaTime);

	// simulation controls
	void ResetSimulation();
	void PlaySimulation();
	void PauseSimulation();
	void IncreaseSimulationSpeed();
	void DecreaseSimulationSpeed();
	void InvertSimulationProgression();			// instead of advancing "forward", the simulation now rolls back
	void MoveToNextSimulationStep();

	// options
	eBVHConstructionStrategy GetCurrenBVHConstructionStrategy() const;
	void SetNewBVHConstructionStrategy(eBVHConstructionStrategy eNewStrategy);

private:
	eBVHConstructionStrategy m_eConstructionStrategy;

	void ClearPreviousVisualization();

	void InitPlaybackSpeeds();
};