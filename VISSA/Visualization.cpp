#include "Visualization.h"

#include <assert.h>
#include <algorithm>
#include <limits>

#include "glm/glm.hpp"

#include "GeometricPrimitiveData.h"

Visualization::Visualization() : 
	m_iMaximumRenderedTreeDepth(),
	m_iNumberStepsRendered(0),
	m_ePresentationMode(DISCRETE),
	m_eConstructionStrategy(TOPDOWN),
	m_uiCurrentPlayBackSpeedIndex(0u),
	m_iSimulationDirectionSign(0),
	m_fAccumulatedTimeSinceLastUpdateStep(0.0f),
	m_bRenderObjectAABBs(false),
	m_bRenderObjectBoundingSpheres(false),
	m_bRenderGridXPlane(false),
	m_bRenderGridYPlane(false),
	m_bRenderGridZPlane(false)
{
	InitPlaybackSpeeds();
	InitRenderColors();
	ResetSimulation();
}

void Visualization::Load()
{
	using namespace Primitives;

	ClearPreviousVisualization();

	std::vector<glm::vec3> vecNewObjectsPositions;
	std::vector<glm::vec3> vecNewObjectsScales;
	std::vector<SceneObject::Transform::Rotation> vecNewObjectsRotations;
	
	vecNewObjectsPositions.reserve(100);
	vecNewObjectsScales.reserve(100);
	vecNewObjectsRotations.reserve(100);

	// make new positions
	vecNewObjectsPositions.push_back(glm::vec3(150.0f, 100.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-200.0f, 100.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(200.0f, 250.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(275.0f, 250.0f, 100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(50.0f, 300.0f, 400.0f));
	vecNewObjectsPositions.push_back(glm::vec3(0.0f, 500.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(300.0f, -300.0f, 300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-300.0f, 250.0f, 500.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-100.0f, 500.0f, -300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-200.0f, 125.0f, -500.0f));

	// make new scales
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
	vecNewObjectsScales.push_back(glm::vec3(0.8f, 0.8f, 0.8f));
	vecNewObjectsScales.push_back(glm::vec3(1.5f, 1.5f, 1.5f));
	vecNewObjectsScales.push_back(glm::vec3(1.7f, 1.7f, 1.7f));
	vecNewObjectsScales.push_back(glm::vec3(0.9f, 0.9f, 0.9f));
	vecNewObjectsScales.push_back(glm::vec3(3.0f, 3.0f, 3.0f));
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(0.2f, 0.2f, 0.2f));

	// make new rotations
	SceneObject::Transform::Rotation tNewRotation;
	tNewRotation.m_fAngle = 45.0f;
	tNewRotation.m_vec3Vector = glm::vec3(0.0f, 0.0f, 1.0f);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);

	assert(vecNewObjectsPositions.size() == vecNewObjectsScales.size());
	assert(vecNewObjectsScales.size() == vecNewObjectsRotations.size());

	for (int uiCurrentNewObject = 0; uiCurrentNewObject < vecNewObjectsPositions.size(); uiCurrentNewObject++)
	//for (int uiCurrentNewObject = 0; uiCurrentNewObject < 4; uiCurrentNewObject++)
	{
		SceneObject tNewObject;
		tNewObject.m_tTransform.m_vec3Position = vecNewObjectsPositions[uiCurrentNewObject];
		tNewObject.m_tTransform.m_tRotation = vecNewObjectsRotations[uiCurrentNewObject];
		tNewObject.m_tTransform.m_vec3Scale = vecNewObjectsScales[uiCurrentNewObject];
		if (uiCurrentNewObject % 2 == 0)
		{
			tNewObject.m_eType = SceneObject::eType::SPHERE;
		}
		else
		{
			tNewObject.m_eType = SceneObject::eType::CUBE;
		}

		m_vecObjects.push_back(tNewObject);
	}
}

void Visualization::Update(float fDeltaTime)
{
	if (m_ePresentationMode == CONTINUOUS)
	{
		const float fPlaybackSpeedAdjustedDeltaTime = fDeltaTime * m_pPlaybackSpeeds[m_uiCurrentPlayBackSpeedIndex]; // at a playback speed of 0.25, time is 4 times slower for the simulation
		m_fAccumulatedTimeSinceLastUpdateStep += fPlaybackSpeedAdjustedDeltaTime;
		if (m_fAccumulatedTimeSinceLastUpdateStep >= 1.0f)
		{
			MoveToNextSimulationStep();
			m_fAccumulatedTimeSinceLastUpdateStep -= 1.0f;
		}
	}
}

void Visualization::ResetSimulation()
{
	m_iNumberStepsRendered = 0;
	m_iMaximumRenderedTreeDepth = 100;
	m_iSimulationDirectionSign = 1;
	m_ePresentationMode = DISCRETE;
	m_uiCurrentPlayBackSpeedIndex = 2u;
}

void Visualization::PlaySimulation()
{
	m_ePresentationMode = CONTINUOUS;
	m_fAccumulatedTimeSinceLastUpdateStep = 0.0f;
}

void Visualization::PauseSimulation()
{
	m_ePresentationMode = DISCRETE;
}

void Visualization::IncreaseSimulationSpeed()
{
	assert(m_uiCurrentPlayBackSpeedIndex <= 4u);
	if (m_uiCurrentPlayBackSpeedIndex < 4u)
		m_uiCurrentPlayBackSpeedIndex++;
}

void Visualization::DecreaseSimulationSpeed()
{
	assert(m_uiCurrentPlayBackSpeedIndex <= 4u);
	if (m_uiCurrentPlayBackSpeedIndex > 0u)
		m_uiCurrentPlayBackSpeedIndex--;
}

void Visualization::InvertSimulationProgression()
{
	m_iSimulationDirectionSign *= -1;
}

void Visualization::MoveToNextSimulationStep()
{
	const int iNextNumberOfConstructionStepsRendered = m_iNumberStepsRendered + m_iSimulationDirectionSign;

	// bounds checks
	m_iNumberStepsRendered = std::max<int>(0, iNextNumberOfConstructionStepsRendered);
	assert(m_vecTreeAABBsForTopDownRendering.size() <= std::numeric_limits<int>::max());	// make sure that number fits or chaos might ensue.
	m_iNumberStepsRendered = std::min<int>(m_iNumberStepsRendered, static_cast<int>(m_vecTreeAABBsForTopDownRendering.size()));
}

Visualization::eBVHConstructionStrategy Visualization::GetCurrenBVHConstructionStrategy() const
{
	return m_eConstructionStrategy;
}

void Visualization::SetNewBVHConstructionStrategy(eBVHConstructionStrategy eNewStrategy)
{
	if (m_eConstructionStrategy != eNewStrategy)
	{
		m_eConstructionStrategy = eNewStrategy;
		ResetSimulation();
	}
}

void Visualization::ClearPreviousVisualization()
{
	m_vecObjects.clear();
}

void Visualization::InitPlaybackSpeeds()
{
	m_pPlaybackSpeeds[0] = 0.25f;
	m_pPlaybackSpeeds[1] = 0.50f;
	m_pPlaybackSpeeds[2] = 1.00f;
	m_pPlaybackSpeeds[3] = 2.00f;
	m_pPlaybackSpeeds[4] = 4.00f;
}

void Visualization::InitRenderColors()
{
	// grid
	m_vec4GridColorX = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4GridColorY = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); // cyan
	m_vec4GridColorZ = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple

	// bounding volumes
	m_vec4AABBDefaultColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4BoundingSphereDefaultColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // blue

	// node colors
	m_vec4TopDownNodeRenderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
	m_vec4BottomUpNodeRenderColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple
}
