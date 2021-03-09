#pragma once

#include <vector>
#include "GeometricPrimitiveData.h"

class Scene {
public:
	

	void LoadScene1();

private:
	std::vector<Primitives::Sphere> m_vecSpheres;
	std::vector<Primitives::Plane> m_vecPlanes;
	std::vector<Primitives::Cube> m_vecCubes;

	void ClearPreviousScene();
};