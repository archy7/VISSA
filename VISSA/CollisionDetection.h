#pragma once

#include "glm/glm.hpp"

class Scene;

namespace CollisionDetection {
	struct AABB {
		glm::vec3 m_vec3Center;
		glm::vec3 m_vec3Radius; // or half-width extents
	};

	struct CollisionDetectionSystem
	{
		AABB m_tTestAABB;
	};

	void ConstructBoundingVolumesForScene(Scene& rScene);
	void UpdateBoundingVolumesForScene(Scene& rScene);

	/*
		pVertices = original vertex data as it was sent to the GPU, including position, normals and UVs (= stride of 8 floats per vertex)

		Constructs a tight-fitting axis-aligned bounding box for the given vertices. 
	*/ 
	AABB ConstructAABBFromVertexData(float* pVertices, size_t uiNumberOfVertices);

	AABB UpdateAABBFromAABB(const AABB& rOldAABB, const glm::mat4& mat4Rotation, const glm::vec3& rTranslation, const glm::vec3& rScale);

	int StaticTestAABBagainstAABB(const AABB& rAABB, const AABB& rOtherAABB);
}