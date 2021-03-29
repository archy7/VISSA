#pragma once

#include "glm/glm.hpp"

#include <vector>

class Visualization;
struct SceneObject;

namespace CollisionDetection {
	struct AABB {
		glm::vec3 m_vec3Center;
		glm::vec3 m_vec3Radius; // aka half-width extents

		float CalcMinimumX() const;
		float CalcMinimumY() const;
		float CalcMinimumZ() const;
		float CalcMaximumX() const;
		float CalcMaximumY() const;
		float CalcMaximumZ() const;

		float CalcMinimumForAxis(size_t uiAxisIndex) const;
		float CalcMaximumForAxis(size_t uiAxisIndex) const;
	};

	struct BoundingSphere {
		glm::vec3 m_vec3Center;
		float m_fRadius = 0.0f;
	};

	struct Ray {
		Ray() {};
		Ray(const glm::vec3& vec3Origin, const glm::vec3& vec3Direction) : // references to avoid unnecessary copies
			m_vec3Origin(vec3Origin),
			m_vec3Direction(vec3Direction)
		{

		};

		glm::vec3 m_vec3Origin;
		glm::vec3 m_vec3Direction;
	};

	struct RayCastIntersectionResult {
		SceneObject* m_pFirstIntersectedSceneObject = nullptr;
		glm::vec3 m_vec3PointOfIntersection;
		float m_fIntersectionDistance = std::numeric_limits<float>::max();

		bool IntersectionWithObjectOccured() const { return m_pFirstIntersectedSceneObject; }
	};

	struct MostSeperatedPointsIndices;

	

	void ConstructBoundingVolumesForScene(Visualization& rScene);
	void UpdateBoundingVolumesForScene(Visualization& rScene);

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
	BoundingSphere UpdateBoundingSphere(const BoundingSphere& rLocalSpaceBB, const glm::vec3& rTranslation, const glm::vec3& rScale);

	/*
		shortcut functions because we know our data.
		Sometimes a little bit of cheating makes a lot of sense.
	*/
	BoundingSphere ConstructLocalSpaceBoundingSphereForCube(const SceneObject& rCurrentCube);
	BoundingSphere ConstructLocalSpaceBoundingSphereForSphere(const SceneObject& rCurrentSphere);

	

	//////////////////////////////////////////////////////////////
	//////////////////////////BVH/////////////////////////////////
	//////////////////////////////////////////////////////////////

	struct BVHTreeNode {
		AABB m_tAABBForNode;
		BoundingSphere m_tBoundingSphereForNode;
		BVHTreeNode* m_pLeft = nullptr;
		BVHTreeNode* m_pRight = nullptr;
		SceneObject* m_pObjects = nullptr;
		uint8_t m_uiNumOjbects = 0u;

		bool IsANode() const {
			return m_pObjects == nullptr;
		}
	};

	struct TreeNodeAABBForRendering {
		BVHTreeNode* m_pNodeToBeRendered;
		int16_t m_iTreeDepth = 0u;
		int16_t m_iRenderingOrder = 0u; // when stepping through the simulation, this determines in which order AABBs are rendered.
	};

	struct BoundingVolumeHierarchy {
		BVHTreeNode* m_pRootNode = nullptr;
		void DeleteTree();
	private:
		void RecursiveDeleteTree(BVHTreeNode* pNode);
	};

	BoundingVolumeHierarchy ConstructTopDownBVHForScene(Visualization& rScene);
	BoundingVolumeHierarchy ConstructBottomUPBVHForScene(Visualization& rScene);
	void RecursiveTopDownBVTree(BVHTreeNode** pNode, SceneObject* pScenObjects, size_t uiNumSceneObjects);
	BVHTreeNode* BottomUpBVTree(SceneObject* pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization);
	void FindBottomUpNodesToMerge(BVHTreeNode** pNode, size_t uiNumNodes, size_t& rNodeIndex1, size_t& rNodeIndex2);
	AABB MergeTwoAABBs(const AABB& rAABB1, const AABB& rAABB2);
	void TraverseTreeForAABBDataForTopDownRendering(BVHTreeNode* pNode, std::vector<TreeNodeAABBForRendering>& rvecAABBsForRendering, int16_t iTreeDepth);
	void TraverseTreeForAABBDataForBottomUpRendering(BVHTreeNode* pNode, std::vector<TreeNodeAABBForRendering>& rvecAABBsForRendering, int16_t iTreeDepth);
	AABB CreateAABBForMultipleObjects(const SceneObject* pScenObjects, size_t uiNumSceneObjects);
	BoundingSphere CreateBoundingSphereForMultipleSceneObjects(const SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	size_t PartitionSceneObjectsInPlace(SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	RayCastIntersectionResult CastRayIntoBVH(const BoundingVolumeHierarchy& rBVH, const Ray& rCastedRay);
	RayCastIntersectionResult RecursiveRayCastIntoBVHTree(const BVHTreeNode* pNode, const Ray& rCastedRay);
	int IntersectRayAABB(const Ray& rIntersectingRay, const AABB& rAABB, float& rfIntersectionDistance, glm::vec3 rvec3IntersectionPoint);
}