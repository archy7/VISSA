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

	void ConstructBoundingVolumesForScene(Visualization& rScene);
	void UpdateBoundingVolumesForScene(Visualization& rScene);
	int StaticTestAABBagainstAABB(const AABB& rAABB, const AABB& rOtherAABB);

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

	struct BoundingVolumeHierarchy {
		//BoundingVolumeHierarchy();
		//~BoundingVolumeHierarchy();
		//BoundingVolumeHierarchy(BoundingVolumeHierarchy& rOther);
		//BoundingVolumeHierarchy& operator=(BoundingVolumeHierarchy tOther);
		BVHTreeNode* m_pRootNode = nullptr;
		int16_t m_iTDeepestDepthOfNodes = 0;

		void DeleteTree();
	private:
		
		void RecursiveDeleteTree(BVHTreeNode* pNode);
	};

	struct TreeNodeForRendering {
		BVHTreeNode* m_pNodeToBeRendered;
		int16_t m_iDepthInTree = 0u;
		int16_t m_iRenderingOrder = 0u; // when stepping through the simulation, this determines in which order node bounding volumes are rendered.
	};

	BoundingVolumeHierarchy ConstructTopDownAABBBVHForScene(Visualization& rScene);
	BoundingVolumeHierarchy ConstructBottomUpAABBBVHForScene(Visualization& rScene);
	BoundingVolumeHierarchy ConstructTopDownBoundingSphereBVHForScene(Visualization& rScene);
	BoundingVolumeHierarchy ConstructBottomUpBoundingSphereBVHForScene(Visualization& rScene);
	RayCastIntersectionResult CastRayIntoBVH(const BoundingVolumeHierarchy& rBVH, const Ray& rCastedRay);
}