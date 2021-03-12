#pragma once

#include "glm/glm.hpp"

namespace CollisionDetection {
	struct AABB {
		glm::vec3 m_vec3Center;
		glm::vec3 m_vec3Radius; // or half-width extents
	};

	struct CollisionDetectionSystem
	{
		AABB m_tTestAABB;
	};

	/*
		pVertices = original vertex data as it was sent to the GPU, including position, normals and UVs (= stride of 8 floats per vertex)

		Constructs a tight-fitting axis-aligned bounding box for the given vertices. 
	*/ 
	AABB MakeAABBFromVertexData(float* pVertices, size_t uiNumberOfVertices);

	int StaticTestAABBagainstAABB(const AABB& rAABB, const AABB& rOtherAABB);
}