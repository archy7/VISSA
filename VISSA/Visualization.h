#pragma once

#include <vector>

#include "SceneObject.h"
#include "CollisionDetection.h"

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

	std::vector<SceneObject> m_vecObjects;
	std::vector<CollisionDetection::TreeNodeAABBForRendering> m_vecTreeAABBsForRendering;
	float m_pPlaybackSpeeds[5]; // in Steps per second
	size_t m_uiCurrentPlayBackSpeedIndex;
	int8_t m_iSimulationDirectionSign; // a value of 1 or -1, 
	float m_fAccumulatedTimeSinceLastUpdateStep;

	int16_t m_iMaximumRenderedTreeDepth;
	int m_iNumberStepsRendered; // the number of Tree node AABBs that is rendered. They are ordered by which was one was contructed first.
	ePresentationMode m_ePresentationMode;
	
	
	void Load();
	void Update(float fDeltaTime);

	void Reset();
	void Play();
	void Pause();
	void IncreaseSpeed();
	void DecreaseSpeed();
	void Invert();			// instead of advancing "forward", the simulation now rolls back
	void MoveToNextStep();


private:
	void ClearPreviousVisualization();

	void InitPlaybackSpeeds();
};