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
		BVHTreeNode* m_pRootNode = nullptr;
		void DeleteTree();
	private:
		void RecursiveDeleteTree(BVHTreeNode* pNode);
	};

	struct TreeNodeForRendering {
		BVHTreeNode* m_pNodeToBeRendered;
		int16_t m_iTreeDepth = 0u;
		int16_t m_iRenderingOrder = 0u; // when stepping through the simulation, this determines in which order AABBs are rendered.
	};

	

	BoundingVolumeHierarchy ConstructTopDownBVHForScene(Visualization& rScene);
	BoundingVolumeHierarchy ConstructBottomUPBVHForScene(Visualization& rScene);
	RayCastIntersectionResult CastRayIntoBVH(const BoundingVolumeHierarchy& rBVH, const Ray& rCastedRay);
}