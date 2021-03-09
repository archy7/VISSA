#include "Scene.h"

#include <assert.h>

#include "glm/glm.hpp"

void Scene::LoadScene1()
{
	using namespace Primitives;

	ClearPreviousScene();

	std::vector<glm::vec3> vecNewSpherePositions;
	std::vector<float> vecNewSphereRadii;

	vecNewSpherePositions.reserve(100);
	vecNewSphereRadii.reserve(100);

	// make new positions
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));
	vecNewSpherePositions.push_back(glm::vec3(0.0f, 150.0f, 100.0f));


	assert(vecNewSpherePositions.size() == vecNewSphereRadii.size());

	// add spheres to the scene
	Sphere tNewSphere;
	tNewSphere.m_vec3Position = glm::vec3(0.0f, 150.0f, 100.0f);
	float fNewSpherePosition;
}

void Scene::ClearPreviousScene()
{

}
