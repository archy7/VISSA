#pragma once

#include <vector>
#include "GeometricPrimitiveData.h"

/*
	This class needs the most reworking to be practically usable. instead of primitives, actual models should be saved here.
	Also, it is common to have "structure of arrays" as opposed to "array of structs"
*/
class Scene {
public:
	struct SceneObject {
		enum eType {
			UNDEFINED = 0,
			SPHERE,
			PLANE,
			CUBE
		} m_eType;

		struct Transform
		{
			glm::vec3 m_vec3Position;
			glm::vec3 m_vec3Scale;
			glm::vec4 m_vec4Rotation;
		} m_tTransform;
	};

	void LoadScene1();
	void LoadScene2();

private:
	std::vector<SceneObject> m_vecObjects;

	void ClearPreviousScene();
};