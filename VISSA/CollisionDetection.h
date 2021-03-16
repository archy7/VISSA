#pragma once

#include "glm/glm.hpp"

class Scene;
struct SceneObject;

namespace CollisionDetection {
	struct AABB {
		glm::vec3 m_vec3Center;
		glm::vec3 m_vec3Radius; // aka half-width extents
	};

	struct BoundingSphere {
		glm::vec3 m_vec3Center;
		float m_fRadius;
	};

	struct MostSeperatedPointsIndices;

	/*struct CollisionDetectionSystem
	{
		AABB m_tTestAABB;
	};*/

	void ConstructBoundingVolumesForScene(Scene& rScene);
	void UpdateBoundingVolumesForScene(Scene& rScene);

	/*
		pVertices = original vertex data as it was sent to the GPU, including position, normals and UVs (= stride of 8 floats per vertex)

		Constructs a tight-fitting axis-aligned bounding box for the given vertices. 
	*/ 
	AABB ConstructAABBFromVertexData(float* pVertices, size_t uiNumberOfVertices);
	
	/*
		Creates a new AABB based on the local space AABB, taking into consideration its new position, scale, and rotation
	*/
	AABB UpdateAABBFromAABB(const AABB& rLocalSpaceAABB, const glm::mat4& mat4Rotation, const glm::vec3& rTranslation, const glm::vec3& rScale);

	int StaticTestAABBagainstAABB(const AABB& rAABB, const AABB& rOtherAABB);

	/*
		Constructs a bounding sphere using the iterative Ritter Approach
	*/
	BoundingSphere ConstructBoundingSphereFromVertexData(float* pVertices, size_t uiNumberOfVertices);
	/*
		Constructs a singular RitterSphere for the given points in local space
	*/
	BoundingSphere ConstructRitterSphere(float* pVertices, size_t uiNumberOfVertices);

	MostSeperatedPointsIndices MostSeperatedPointsOnAABB(float* pVertices, size_t uiNumberOfVertices);
	BoundingSphere BoundingSphereFromDistantPoints(float* pVertices, size_t uiNumberOfVertices);
	void UpdateSphereToEncompassPoint(BoundingSphere& rSphereToBeUpdated, const glm::vec3& rvec3PointToBeEncompassed);

	/*
		shortcut functions because we know our data.
		Sometimes a little bit of cheating makes a lot of sense.
	*/
	BoundingSphere ConstructLocalSpaceBoundingSphereForCube(const SceneObject& rCurrentCube);
	BoundingSphere ConstructLocalSpaceBoundingSphereForSphere(const SceneObject& rCurrentSphere);

	BoundingSphere UpdateBoundingSphere(const BoundingSphere& rLocalSpaceBB, const glm::vec3& rTranslation, const glm::vec3& rScale);
}