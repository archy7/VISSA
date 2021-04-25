#pragma once

#include "glm/glm.hpp"

#include <vector>

class Visualization;
struct SceneObject;
class Scene;


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

		float CalcMinimumX() const;
		float CalcMinimumY() const;
		float CalcMinimumZ() const;
		float CalcMaximumX() const;
		float CalcMaximumY() const;
		float CalcMaximumZ() const;
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

	void ConstructBoundingVolumesForScene(Scene & rScene);
	void UpdateBoundingVolumesForScene(Scene& rScene);
	int StaticTestAABBagainstAABB(const AABB& rAABB, const AABB& rOtherAABB);
	AABB CreateAABBForMultipleObjects(const SceneObject * pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	AABB MergeTwoAABBs(const AABB& rAABB1, const AABB& rAABB2);
	/*
		TODO: DOC
	*/
	BoundingSphere CreateBoundingSphereForMultipleObjects(const SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	BoundingSphere MergeTwoBoundingSpheres(const BoundingSphere& rBoundingSphere1, const BoundingSphere& rBoundingSphere2);

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

	struct BoundingVolumeHierarchy {			// todo: turn this into a class with a destructor that ensures deletion of tree
		BVHTreeNode* m_pRootNode = nullptr;
		int16_t m_iTDeepestDepthOfNodes = 0;

		void DeleteTree();
	private:
		
		void RecursiveDeleteTree(BVHTreeNode* pNode);
	};

	/*
		TODO: DOC
	*/
	size_t PartitionSceneObjectsInPlace_AABB(SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	size_t PartitionSceneObjectsInPlace_BoundingSphere(SceneObject* pSceneObjects, size_t uiNumSceneObjects);
	/*
		TODO: DOC
	*/
	void FindBottomUpNodesToMerge_AABB(BVHTreeNode** pNode, size_t uiNumNodes, size_t& rNodeIndex1, size_t& rNodeIndex2);
	/*
		TODO: DOC
	*/
	void FindBottomUpNodesToMerge_BoundingSphere(BVHTreeNode** pNode, size_t uiNumNodes, size_t& rNodeIndex1, size_t& rNodeIndex2);

	RayCastIntersectionResult CastRayIntoBVH(const BoundingVolumeHierarchy& rBVH, const Ray& rCastedRay);
	RayCastIntersectionResult BruteForceRayIntoObjects(std::vector<SceneObject>& rvecObjects, const Ray& rCastedRay);
}