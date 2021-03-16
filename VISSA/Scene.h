#pragma once

#include <vector>

#include "SceneObject.h"

/*
	This class needs the most reworking to be practically usable. instead of primitives, actual models should be saved here.
	Also, it is common to have "structure of arrays" as opposed to "array of structs"
*/
class Scene {
public:
	

	std::vector<SceneObject> m_vecObjects;

	void LoadScene1();
	void LoadScene2();

private:
	void ClearPreviousScene();
};