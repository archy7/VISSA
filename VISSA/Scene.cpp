#include "Scene.h"

#include <assert.h>

#include "glm/glm.hpp"

void Scene::LoadScene1()
{
	using namespace Primitives;

	ClearPreviousScene();

	std::vector<glm::vec3> vecNewObjectsPositions;
	std::vector<glm::vec3> vecNewObjectsScales;
	std::vector<SceneObject::Transform::Rotation> vecNewObjectsRotations;
	
	vecNewObjectsPositions.reserve(100);
	vecNewObjectsScales.reserve(100);
	vecNewObjectsRotations.reserve(100);

	// make new positions
	vecNewObjectsPositions.push_back(glm::vec3(0.0f, 150.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(100.0f, -150.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(200.0f, 250.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(75.0f, 250.0f, 100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(50.0f, 300.0f, 100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(0.0f, 500.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(300.0f, -300.0f, 300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-300.0f, 250.0f, 500.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-100.0f, 500.0f, -300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-200.0f, 125.0f, -500.0f));

	// make new scales
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
	vecNewObjectsScales.push_back(glm::vec3(1.0f, 1.0f, 1.0f));

	// make new rotations
	SceneObject::Transform::Rotation tNewRotation;
	tNewRotation.m_fAngle = 0.0f;
	tNewRotation.m_vec3Vector = glm::vec3(0.0f, 1.0f, 0.0f);
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
	{
		SceneObject tNewObject;
		tNewObject.m_tTransform.m_vec3Position = vecNewObjectsPositions[uiCurrentNewObject];
		tNewObject.m_tTransform.m_tRotation = vecNewObjectsRotations[uiCurrentNewObject];
		tNewObject.m_tTransform.m_vec3Scale = vecNewObjectsScales[uiCurrentNewObject];
		tNewObject.m_eType = (uiCurrentNewObject %2 == 0) ? SceneObject::eType::SPHERE : SceneObject::eType::CUBE;

		m_vecObjects.push_back(tNewObject);
	}
}

void Scene::LoadScene2()
{
	assert(!"nope");
}

void Scene::ClearPreviousScene()
{
	m_vecObjects.clear();
}
