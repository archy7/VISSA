#pragma once

#include "glm/glm.hpp"
#include "CollisionDetection.h"

struct SceneObject {
	enum eType {
		UNDEFINED = 0,
		SPHERE,
		PLANE,
		CUBE
	} m_eType = CUBE;

	struct Transform
	{
		glm::vec3 m_vec3Position = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 m_vec3Scale = glm::vec3(1.0f, 1.0f, 1.0f);
		struct Rotation {
			glm::vec3 m_vec3Axis = glm::vec3(1.0f, 1.0f, 1.0f);
			float m_fAngle = 0.0f;
		} m_tRotation;

	} m_tTransform;

	CollisionDetection::AABB m_tLocalSpaceAABB;
	CollisionDetection::AABB m_tWorldSpaceAABB;
	CollisionDetection::BoundingSphere m_tLocalSpaceBoundingSphere;
	CollisionDetection::BoundingSphere m_tWorldSpaceBoundingSphere;
};