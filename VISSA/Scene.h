#pragma once

#include <vector>

#include "SceneObject.h"

/*
	todo: this class will need to be more elaborate in the future, 
	i.e. scene loading from file
	struct of arrays
*/
class Scene {
public:
	
//private:
	std::vector<SceneObject> m_vecObjects;
};