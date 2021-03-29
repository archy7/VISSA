#pragma once

#include "glm/glm.hpp"
#include "CollisionDetection.h"

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
		struct Rotation {
			glm::vec3 m_vec3Axis;
			float m_fAngle;
		} m_tRotation;

	} m_tTransform;

	CollisionDetection::AABB m_tLocalSpaceAABB;
	CollisionDetection::AABB m_tWorldSpaceAABB;
	CollisionDetection::BoundingSphere m_tLocalSpaceBoundingSphere;
	CollisionDetection::BoundingSphere m_tWorldSpaceBoundingSphere;
};