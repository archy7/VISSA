#include "CollisionDetection.h"

#include <limits>
#include <algorithm>

#include "Visualization.h"
#include "SceneObject.h"
#include "GeometricPrimitiveData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace CollisionDetection;	// ok here since we are in a translation unit devoted to the CollisionDetection namespace

/*
	Forward declaration of "private" functions and types (internal linkage)
*/
namespace CollisionDetection {
	namespace {
		//////////////////////////////////////////
		// TYPES
		//////////////////////////////////////////
		struct MostSeperatedPointsIndices {
			size_t m_uiMinVertexIndex;
			size_t m_uiMaxVertexIndex;
		};

		//////////////////////////////////////////
		// BOUNDING VOLUMES
		//////////////////////////////////////////

		/*
			pVertices = original vertex data as it was sent to the GPU, including position, normals and UVs (= stride of 8 floats per vertex)

			Constructs a tight-fitting axis-aligned bounding box for the given vertices.
		*/
		AABB ConstructAABBFromVertexData(float* pVertices, size_t uiNumberOfVertices);
		/*
			Creates a new AABB based on the local space AABB, taking into consideration its new position, UNIFORM scale, and rotation.
			Uniform scaling can easily be supported by the common and efficient way of updating an AABB.
			Here, updating is performed on a per axis basis.
		*/
		AABB UpdateAABBFromAABB_UniformScaling(const AABB& rLocalSpaceAABB, const glm::mat4& mat4Rotation, const glm::vec3& rTranslation, const glm::vec3& rScale);
		/*
			Creates a new AABB based on the local space AABB, taking into consideration its new position, NON-UNIFORM scale, and rotation.
			In order to support non-uniform scaling, a more expensive approach to updating AABBs is required.
			Here, updating is performed on a per "corner point of AABB" basis.
		*/
		AABB UpdateAABBFromAABB_NonUniformScaling(const AABB& rLocalSpaceAABB, const glm::mat4& mat4Transform);
		/*
			Constructs a bounding sphere using the iterative Ritter Approach
		*/
		BoundingSphere ConstructBoundingSphereFromVertexData_Iterative(float* pVertices, size_t uiNumberOfVertices);
		/*
			Constructs a singular RitterSphere for the given points in local space
		*/
		BoundingSphere ConstructBoundingSphereFromVertexData(float* pVertices, size_t uiNumberOfVertices);
		/*
			TODO: DOC
		*/
		MostSeperatedPointsIndices MostSeperatedPointsOnAABB(float * pVertices, size_t uiNumberOfVertices);
		/*
			TODO: doc
		*/
		BoundingSphere BoundingSphereFromDistantPoints(float* pVertices, size_t uiNumberOfVertices);
		/*
			TODO: doc
		*/
		void ConditionallyUpdateSphereToEncompassPoint(BoundingSphere& rSphereToBeUpdated, const glm::vec3& rvec3PointToBeEncompassed);
		/*
			TODO: doc
		*/
		BoundingSphere UpdateBoundingSphere(const BoundingSphere& rLocalSpaceBB, const glm::vec3& rTranslation, const glm::vec3& rScale);
		/*
			cheating shortcut function that creates a Bounding Sphere for an object by exploiting intrisic knowledge that the object is a cube.
		*/
		BoundingSphere ConstructLocalSpaceBoundingSphereForCube(const SceneObject& rCurrentCube);
		/*
			cheating shortcut function that creates a Bounding Sphere for an object by exploiting intrisic knowledge that the object is a sphere.
		*/
		BoundingSphere ConstructLocalSpaceBoundingSphereForSphere(const SceneObject& rCurrentSphere);

		//////////////////////////////////////////
		// BOUNDING VOLUME HIERARCHY
		//////////////////////////////////////////

		/*
			recursive function that constructs a top down AABB tree
		*/
		void RecursiveTopDownTree_AABB(BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
		/*
			TODO: DOC
		*/
		BVHTreeNode* BottomUpTree_AABB(SceneObject* pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization);
		/*
			TODO: DOC
		*/
		void FindBottomUpNodesToMerge_AABB(BVHTreeNode** pNode, size_t uiNumNodes, size_t& rNodeIndex1, size_t& rNodeIndex2);
		/*
			TODO: DOC
		*/
		AABB MergeTwoAABBs(const AABB& rAABB1, const AABB& rAABB2);
		/*
			TODO: DOC
		*/
		void TraverseTreeForDataForTopDownRendering_AABB(BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecAABBsForRendering, int16_t iTreeDepth, int16_t& riTotalTreeDepth);
		/*
			TODO: DOC
		*/
		void TraverseTreeForDataForBottomUpRendering_AABB(BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecAABBsForRendering, int16_t iTreeDepth, int16_t& riTotalTreeDepth);
		/*
			TODO: DOC
		*/
		AABB CreateAABBForMultipleObjects(const SceneObject* pSceneObjects, size_t uiNumSceneObjects);
		/*
			TODO: DOC
		*/
		size_t PartitionSceneObjectsInPlace_AABB(SceneObject* pSceneObjects, size_t uiNumSceneObjects);


		/*
			recursive function that constructs a top down Bounding Sphere tree
		*/
		void RecursiveTopDownTree_BoundingSphere(BVHTreeNode** pNode, SceneObject* pSceneObjects, size_t uiNumSceneObjects);
		/*
			TODO: DOC
		*/
		BVHTreeNode* BottomUpTree_BoundingSphere(SceneObject* pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization);
		/*
			TODO: DOC
		*/
		void FindBottomUpNodesToMerge_BoundingSphere(BVHTreeNode** pNode, size_t uiNumNodes, size_t& rNodeIndex1, size_t& rNodeIndex2);
		/*
			TODO: DOC
		*/
		BoundingSphere MergeTwoBoundingSpheres(const BoundingSphere& rBoundingSphere1, const BoundingSphere& rBoundingSphere2);
		/*
			TODO: DOC
		*/
		void TraverseTreeForDataForTopDownRendering_BoundingSphere(BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecBoundingspheresForRendering, int16_t iTreeDepth, int16_t& riTotalTreeDepth);
		/*
			TODO: DOC
		*/
		void TraverseTreeForDataForBottomUpRendering_BoundingSphere(BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecBoundingSpheresForRendering, int16_t iTreeDepth, int16_t& riTotalTreeDepth);
		/*
			TODO: DOC
		*/
		BoundingSphere CreateBoundingSphereForMultipleObjects(const SceneObject* pSceneObjects, size_t uiNumSceneObjects);
		/*
			TODO: DOC
		*/
		size_t PartitionSceneObjectsInPlace_BoundingSphere(SceneObject* pSceneObjects, size_t uiNumSceneObjects);

		//////////////////////////////////////////
		// RAY CASTING
		//////////////////////////////////////////

		/*
			TODO: DOC
		*/
		RayCastIntersectionResult RecursiveRayCastIntoBVHTree(const BVHTreeNode* pNode, const Ray& rCastedRay);
		/*
			TODO: DOC
		*/
		int IntersectRayAABB(const Ray& rIntersectingRay, const AABB& rAABB, float& rfIntersectionDistance, glm::vec3 rvec3IntersectionPoint);
	}
	
};

/*
	implementation of "public" functions (external linkage)
*/

float CollisionDetection::AABB::CalcMinimumX() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.x - m_vec3Radius.x;;
}

float CollisionDetection::AABB::CalcMinimumY() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.y - m_vec3Radius.y;
}

float CollisionDetection::AABB::CalcMinimumZ() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.z - m_vec3Radius.z;
}

float CollisionDetection::AABB::CalcMaximumX() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.x + m_vec3Radius.x;
}

float CollisionDetection::AABB::CalcMaximumY() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.y + m_vec3Radius.y;
}

float CollisionDetection::AABB::CalcMaximumZ() const
{
	assert(glm::length(m_vec3Radius) > 0.0f);

	return m_vec3Center.z + m_vec3Radius.z;
}

float CollisionDetection::AABB::CalcMinimumForAxis(size_t uiAxisIndex) const
{
	assert(glm::length(m_vec3Radius) > 0.0f);
	assert(uiAxisIndex <= 2u);	// logic
	assert(uiAxisIndex <= std::numeric_limits<glm::vec3::length_type>::max()); // technical

	const auto uiNonWarningProducingAxisIndex = static_cast<glm::vec3::length_type>(uiAxisIndex);

	return m_vec3Center[uiNonWarningProducingAxisIndex] - m_vec3Radius[uiNonWarningProducingAxisIndex];
}

float CollisionDetection::AABB::CalcMaximumForAxis(size_t uiAxisIndex) const
{
	assert(glm::length(m_vec3Radius) > 0.0f);
	assert(uiAxisIndex <= 2u);	// logic
	assert(uiAxisIndex <= std::numeric_limits<glm::vec3::length_type>::max()); // technical

	const auto uiNonWarningProducingAxisIndex = static_cast<glm::vec3::length_type>(uiAxisIndex);

	return m_vec3Center[uiNonWarningProducingAxisIndex] + m_vec3Radius[uiNonWarningProducingAxisIndex];
}

float CollisionDetection::BoundingSphere::CalcMinimumX() const
{
	return m_vec3Center.x - m_fRadius;
}

float CollisionDetection::BoundingSphere::CalcMinimumY() const
{
	return m_vec3Center.y - m_fRadius;
}

float CollisionDetection::BoundingSphere::CalcMinimumZ() const
{
	return m_vec3Center.z - m_fRadius;
}

float CollisionDetection::BoundingSphere::CalcMaximumX() const
{
	return m_vec3Center.x + m_fRadius;
}

float CollisionDetection::BoundingSphere::CalcMaximumY() const
{
	return m_vec3Center.y + m_fRadius;
}

float CollisionDetection::BoundingSphere::CalcMaximumZ() const
{
	return m_vec3Center.z + m_fRadius;
}

void CollisionDetection::ConstructBoundingVolumesForScene(Visualization& rScene)
{
	for (SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		if (rCurrentSceneObject.m_eType == SceneObject::eType::CUBE)
		{
			rCurrentSceneObject.m_tLocalSpaceAABB = ConstructAABBFromVertexData(Primitives::Cube::VertexData, sizeof(Primitives::Cube::IndexData) / sizeof(GLfloat));
			//rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructBoundingSphereFromVertexData(Primitives::Cube::VertexData, sizeof(Primitives::Cube::IndexData) / sizeof(GLfloat));
			rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructLocalSpaceBoundingSphereForCube(rCurrentSceneObject);

		}
		else if(rCurrentSceneObject.m_eType == SceneObject::eType::SPHERE)
		{
			rCurrentSceneObject.m_tLocalSpaceAABB = ConstructAABBFromVertexData(Primitives::Sphere::VertexData, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
			rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructBoundingSphereFromVertexData(Primitives::Sphere::VertexData, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
			//rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructLocalSpaceBoundingSphereForSphere(rCurrentSceneObject);
		}
		else
		{
			assert(!"nothing here!");
		}
	}
}

void CollisionDetection::UpdateBoundingVolumesForScene(Visualization & rScene)
{
	for (SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		const SceneObject::Transform& rCurrentObjectTransform = rCurrentSceneObject.m_tTransform;

		if (rCurrentObjectTransform.HasUniformScaling())
		{
			glm::mat4 mat4Transform = glm::mat4(1.0f); // identity
			mat4Transform = glm::rotate(mat4Transform, glm::radians(rCurrentObjectTransform.m_tRotation.m_fAngle), rCurrentObjectTransform.m_tRotation.m_vec3Axis);

			rCurrentSceneObject.m_tWorldSpaceAABB = UpdateAABBFromAABB_UniformScaling(rCurrentSceneObject.m_tLocalSpaceAABB, mat4Transform, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
		}
		else
		{
			glm::mat4 mat4Transform = glm::mat4(1.0f); // identity
			mat4Transform = glm::translate(mat4Transform, rCurrentObjectTransform.m_vec3Position);
			mat4Transform = glm::rotate(mat4Transform, glm::radians(rCurrentObjectTransform.m_tRotation.m_fAngle), rCurrentObjectTransform.m_tRotation.m_vec3Axis);
			mat4Transform = glm::scale(mat4Transform, rCurrentObjectTransform.m_vec3Scale);

			rCurrentSceneObject.m_tWorldSpaceAABB = UpdateAABBFromAABB_NonUniformScaling(rCurrentSceneObject.m_tLocalSpaceAABB, mat4Transform);
		}
		
		// updated Bounding Sphere
		rCurrentSceneObject.m_tWorldSpaceBoundingSphere = UpdateBoundingSphere(rCurrentSceneObject.m_tLocalSpaceBoundingSphere, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
	}
}

int CollisionDetection::StaticTestAABBagainstAABB(const AABB & rAABB, const AABB & rOtherAABB)
{
	if (std::abs(rAABB.m_vec3Center.x - rOtherAABB.m_vec3Center.x) > (rAABB.m_vec3Radius.x + rOtherAABB.m_vec3Radius.x))
		return 0;
	if (std::abs(rAABB.m_vec3Center.y - rOtherAABB.m_vec3Center.y) > (rAABB.m_vec3Radius.y + rOtherAABB.m_vec3Radius.y))
		return 0;
	if (std::abs(rAABB.m_vec3Center.z - rOtherAABB.m_vec3Center.z) > (rAABB.m_vec3Radius.z + rOtherAABB.m_vec3Radius.z))
		return 0;
	return 1;
}

	//////////////////////////////////////////////////////////////
	//////////////////////////BVH/////////////////////////////////
	//////////////////////////////////////////////////////////////

CollisionDetection::BoundingVolumeHierarchy CollisionDetection::ConstructTopDownAABBBVHForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BoundingVolumeHierarchy tResult;
	
	// the construction
	tResult.m_pRootNode = new BVHTreeNode;
	RecursiveTopDownTree_AABB(&(tResult.m_pRootNode), rScene.m_vecObjects.data(), rScene.m_vecObjects.size());

	// first traversal to gather data for rendering. In theory, it is possible to traverse the tree every frame for BV rendering.
	// But that is terrible, so data is fetched into a linear vector
	rScene.m_vecTreeAABBsForTopDownRendering.clear();
	rScene.m_vecTreeAABBsForTopDownRendering.reserve(rScene.m_vecObjects.size());
	TraverseTreeForDataForTopDownRendering_AABB(tResult.m_pRootNode, rScene.m_vecTreeAABBsForTopDownRendering, 0, tResult.m_iTDeepestDepthOfNodes);

	return tResult;
}

BoundingVolumeHierarchy CollisionDetection::ConstructBottomUpAABBBVHForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BoundingVolumeHierarchy tResult;
	rScene.m_vecTreeAABBsForBottomUpRendering.clear();
	rScene.m_vecTreeAABBsForBottomUpRendering.reserve(rScene.m_vecObjects.size());

	// the construction INCLUDING HALF THE PREPARATION OF AABB RENDERING DATA
	tResult.m_pRootNode = BottomUpTree_AABB(rScene.m_vecObjects.data(), rScene.m_vecObjects.size(), rScene);
	TraverseTreeForDataForBottomUpRendering_AABB(tResult.m_pRootNode, rScene.m_vecTreeAABBsForBottomUpRendering, 0, tResult.m_iTDeepestDepthOfNodes);

	return tResult;
}

BoundingVolumeHierarchy CollisionDetection::ConstructTopDownBoundingSphereBVHForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BoundingVolumeHierarchy tResult;

	// the construction
	tResult.m_pRootNode = new BVHTreeNode;
	RecursiveTopDownTree_BoundingSphere(&(tResult.m_pRootNode), rScene.m_vecObjects.data(), rScene.m_vecObjects.size());

	// first traversal to gather data for rendering. In theory, it is possible to traverse the tree every frame for BV rendering.
	// But that is terrible, so data is fetched into a linear vector
	rScene.m_vecTreeBoundingSpheresForTopDownRendering.clear();
	rScene.m_vecTreeBoundingSpheresForTopDownRendering.reserve(rScene.m_vecObjects.size());
	TraverseTreeForDataForTopDownRendering_BoundingSphere(tResult.m_pRootNode, rScene.m_vecTreeBoundingSpheresForTopDownRendering, 0, tResult.m_iTDeepestDepthOfNodes);

	return tResult;
}

BoundingVolumeHierarchy CollisionDetection::ConstructBottomUpBoundingSphereBVHForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BoundingVolumeHierarchy tResult;
	rScene.m_vecTreeBoundingSpheresForBottomUpRendering.clear();
	rScene.m_vecTreeBoundingSpheresForBottomUpRendering.reserve(rScene.m_vecObjects.size());

	// the construction INCLUDING HALF THE PREPARATION OF BOUNDING SPHERE RENDERING DATA
	tResult.m_pRootNode = BottomUpTree_BoundingSphere(rScene.m_vecObjects.data(), rScene.m_vecObjects.size(), rScene);
	TraverseTreeForDataForBottomUpRendering_BoundingSphere(tResult.m_pRootNode, rScene.m_vecTreeBoundingSpheresForBottomUpRendering, 0, tResult.m_iTDeepestDepthOfNodes);

	return tResult;
}

void CollisionDetection::BoundingVolumeHierarchy::DeleteTree()
{
	RecursiveDeleteTree(m_pRootNode);
	delete m_pRootNode;
	m_pRootNode = nullptr;
}

void CollisionDetection::BoundingVolumeHierarchy::RecursiveDeleteTree(BVHTreeNode * pNode)
{
	if (pNode == nullptr)
		return;

	if (pNode->m_pLeft)
	{
		RecursiveDeleteTree(pNode->m_pLeft);
	}

	if (pNode->m_pRight)
	{
		RecursiveDeleteTree(pNode->m_pRight);
	}

	delete pNode->m_pLeft;
	pNode->m_pLeft = nullptr;
	delete pNode->m_pRight;
	pNode->m_pRight = nullptr;
}

//////////////////////////////////////////
// RAY CASTING
//////////////////////////////////////////

RayCastIntersectionResult CollisionDetection::CastRayIntoBVH(const BoundingVolumeHierarchy & rBVH, const Ray & rCastedRay)
{
	RayCastIntersectionResult tResult;

	if (rBVH.m_pRootNode) // only actually cast a ray if there are objects in the scene
	{
		tResult = RecursiveRayCastIntoBVHTree(rBVH.m_pRootNode, rCastedRay);
	}

	return tResult;
}

/*
	Implementation of "private" functions (internal linkage)
*/
namespace CollisionDetection {
	namespace {

		//////////////////////////////////////////
		// BOUNDING VOLUMES
		//////////////////////////////////////////

		MostSeperatedPointsIndices MostSeperatedPointsOnAABB(float * pVertices, size_t uiNumberOfVertices)
		{
			MostSeperatedPointsIndices tResult;

			size_t uiMinXIndex = 0u, uiMaxXIndex = 0u, uiMinYIndex = 0u, uiMaxYIndex = 0u, uiMinZIndex = 0u, uiMaxZIndex = 0u;
			for (size_t uiCurrentVertexIndex = 1; uiCurrentVertexIndex < uiNumberOfVertices; uiCurrentVertexIndex++)
			{
				//min and max for X: *8 accounts for stride, +0 accounts for x coordinate
				if (pVertices[uiCurrentVertexIndex * 8 + 0] < pVertices[uiMinXIndex * 8 + 0]) uiMinXIndex = uiCurrentVertexIndex;
				if (pVertices[uiCurrentVertexIndex * 8 + 0] > pVertices[uiMaxXIndex * 8 + 0]) uiMaxXIndex = uiCurrentVertexIndex;
				//min and max for Y: *8 accounts for stride, +1 accounts for y coordinate
				if (pVertices[uiCurrentVertexIndex * 8 + 1] < pVertices[uiMinYIndex * 8 + 1]) uiMinYIndex = uiCurrentVertexIndex;
				if (pVertices[uiCurrentVertexIndex * 8 + 1] > pVertices[uiMaxYIndex * 8 + 1]) uiMaxYIndex = uiCurrentVertexIndex;
				//min and max for Y: *8 accounts for stride, +2 accounts for z coordinate
				if (pVertices[uiCurrentVertexIndex * 8 + 2] < pVertices[uiMinZIndex * 8 + 2]) uiMinZIndex = uiCurrentVertexIndex;
				if (pVertices[uiCurrentVertexIndex * 8 + 2] > pVertices[uiMaxZIndex * 8 + 2]) uiMaxZIndex = uiCurrentVertexIndex;
			}

			//creating points for distance calculation
			const glm::vec3 vec3MinXPoint(pVertices[uiMinXIndex * 8 + 0], pVertices[uiMinXIndex * 8 + 1], pVertices[uiMinXIndex * 8 + 2]);
			const glm::vec3 vec3MaxXPoint(pVertices[uiMaxXIndex * 8 + 0], pVertices[uiMaxXIndex * 8 + 1], pVertices[uiMaxXIndex * 8 + 2]);
			const glm::vec3 vec3MinYPoint(pVertices[uiMinYIndex * 8 + 0], pVertices[uiMinYIndex * 8 + 1], pVertices[uiMinYIndex * 8 + 2]);
			const glm::vec3 vec3MaxYPoint(pVertices[uiMaxYIndex * 8 + 0], pVertices[uiMaxYIndex * 8 + 1], pVertices[uiMaxYIndex * 8 + 2]);
			const glm::vec3 vec3MinZPoint(pVertices[uiMinZIndex * 8 + 0], pVertices[uiMinZIndex * 8 + 1], pVertices[uiMinZIndex * 8 + 2]);
			const glm::vec3 vec3MaxZPoint(pVertices[uiMaxZIndex * 8 + 0], pVertices[uiMaxZIndex * 8 + 1], pVertices[uiMaxZIndex * 8 + 2]);

			//calculating the distace
			const glm::vec3 vec3DistanceXPoints = vec3MaxXPoint - vec3MinXPoint;
			const glm::vec3 vec3DistanceYPoints = vec3MaxYPoint - vec3MinYPoint;
			const glm::vec3 vec3DistanceZPoints = vec3MaxZPoint - vec3MinZPoint;

			//computing the squared distance for all pairs
			const float fSquaredDistanceXPoints = glm::dot(vec3DistanceXPoints, vec3DistanceXPoints);
			const float fSquaredDistanceYPoints = glm::dot(vec3DistanceYPoints, vec3DistanceYPoints);
			const float fSquaredDistanceZPoints = glm::dot(vec3DistanceZPoints, vec3DistanceZPoints);

			//references only for readability
			size_t& ruiMinVertexIndex = tResult.m_uiMinVertexIndex;
			size_t& ruiMaxVertexIndex = tResult.m_uiMaxVertexIndex;

			//base assumption : arbitrary axis, here X, is the longest
			ruiMinVertexIndex = uiMinXIndex;
			ruiMaxVertexIndex = uiMaxXIndex;

			//correction into Y axis if those points are farther apart
			if (fSquaredDistanceYPoints > fSquaredDistanceXPoints && fSquaredDistanceYPoints > fSquaredDistanceZPoints)
			{
				ruiMinVertexIndex = uiMinYIndex;
				ruiMaxVertexIndex = uiMaxYIndex;
			}

			//correction into Z axis if those points are farther apart
			if (fSquaredDistanceZPoints > fSquaredDistanceXPoints && fSquaredDistanceZPoints > fSquaredDistanceYPoints)
			{
				ruiMinVertexIndex = uiMinZIndex;
				ruiMaxVertexIndex = uiMaxZIndex;
			}

			return tResult;
		}

		AABB ConstructAABBFromVertexData(float * pVertices, size_t uiNumberOfVertices)
		{
			AABB tResult;

			// initializing min values to max and vice versa for definitive overwriting for the first vertex
			float fXMin = std::numeric_limits<float>::max();
			float fXMax = std::numeric_limits<float>::lowest();
			float fYMin = std::numeric_limits<float>::max();
			float fYMax = std::numeric_limits<float>::lowest();
			float fZMin = std::numeric_limits<float>::max();
			float fZMax = std::numeric_limits<float>::lowest();

			for (size_t uiCurrentVertex = 0u; uiCurrentVertex < uiNumberOfVertices; uiCurrentVertex++)
			{
				const size_t uiCurrentDataIndex = uiCurrentVertex * 8; // 8 is the assumed stride;

				const float fVertexPositionX = pVertices[uiCurrentDataIndex + 0];
				const float fVertexPositionY = pVertices[uiCurrentDataIndex + 1];
				const float fVertexPositionZ = pVertices[uiCurrentDataIndex + 2];

				fXMin = std::min<float>(fXMin, fVertexPositionX);
				fYMin = std::min<float>(fYMin, fVertexPositionY);
				fZMin = std::min<float>(fZMin, fVertexPositionZ);
				fXMax = std::max<float>(fXMax, fVertexPositionX);
				fYMax = std::max<float>(fYMax, fVertexPositionY);
				fZMax = std::max<float>(fZMax, fVertexPositionZ);
			}

			/*
				maximum and minimum extents along current axis are now stored
				now to determine the radius of the AABB.
				to tightly fit the AABB for the given vertices, the absolute distance to the centre, which is assumed to be 0,0,0
				are compared. The longer distance is the half-width for that axis.
			*/

			const float fHalfWidthX = std::max(std::abs(fXMin), std::abs(fXMax));
			const float fHalfWidthY = std::max(std::abs(fYMin), std::abs(fYMax));
			const float fHalfWidthZ = std::max(std::abs(fZMin), std::abs(fZMax));

			tResult.m_vec3Center = glm::vec3(0.0f, 0.0f, 0.0f);		// local space center / pivot point. assumed to be 0,0,0. This is true for all of VISSAs objects
			tResult.m_vec3Radius = glm::vec3(fHalfWidthX, fHalfWidthY, fHalfWidthZ);

			return tResult;
		}

		AABB UpdateAABBFromAABB_UniformScaling(const AABB& rLocalSpaceAABB, const glm::mat4& mat4Rotation, const glm::vec3& rTranslation, const glm::vec3& rScale)
		{
			assert(glm::length(rLocalSpaceAABB.m_vec3Radius) > 0.0f);	// make sure old AABB has already been constructed

			AABB tResult;

			for (int i = 0; i < 3; i++) // for every axis of the new AABB
			{
				tResult.m_vec3Center[i] = rTranslation[i];	// we assume the translation
				tResult.m_vec3Radius[i] = 0.0f;				// and start with a radius of 0
				for (int j = 0; j < 3; j++)		// for every axis of the old AABB
				{
					tResult.m_vec3Center[i] += mat4Rotation[i][j] * rLocalSpaceAABB.m_vec3Center[j];				// we adjust the center. This only has an effect, when the old AABBs center was not {0,0,0}, like somewhere in world-space.	When the old AABBs centre point was in 0,0,0 local space, this will do nothing.
					tResult.m_vec3Radius[i] += std::abs(mat4Rotation[i][j]) * rLocalSpaceAABB.m_vec3Radius[j];		// we continuosly increase the radius, starting at 0. For every axis of the old, rotated old AABB (j), we add its impact to the radius of the axis of the new AABB (i)
				}
				tResult.m_vec3Radius[i] *= rScale[i]; // we scale the current axis of the new AABB
			}

			return tResult;
		}

		AABB UpdateAABBFromAABB_NonUniformScaling(const AABB & rLocalSpaceAABB, const glm::mat4 & mat4Transform)
		{
			assert(glm::length(rLocalSpaceAABB.m_vec3Radius) > 0.0f);	// make sure old AABB has already been constructed

			AABB tResult;

			// world space AABB center by extracting the 4th colum of the transform matrix
			const glm::vec3 vec3NewAABBCenter = mat4Transform[3];
			tResult.m_vec3Center = vec3NewAABBCenter;

			// constructing all corner points of the old AABB
			std::vector<glm::vec4> vecOldAABBCornerPoints{
				glm::vec4(rLocalSpaceAABB.CalcMaximumX(), rLocalSpaceAABB.CalcMaximumY(), rLocalSpaceAABB.CalcMaximumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMinimumX(), rLocalSpaceAABB.CalcMaximumY(), rLocalSpaceAABB.CalcMaximumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMaximumX(), rLocalSpaceAABB.CalcMinimumY(), rLocalSpaceAABB.CalcMaximumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMaximumX(), rLocalSpaceAABB.CalcMaximumY(), rLocalSpaceAABB.CalcMinimumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMinimumX(), rLocalSpaceAABB.CalcMinimumY(), rLocalSpaceAABB.CalcMaximumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMaximumX(), rLocalSpaceAABB.CalcMinimumY(), rLocalSpaceAABB.CalcMinimumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMinimumX(), rLocalSpaceAABB.CalcMaximumY(), rLocalSpaceAABB.CalcMinimumZ(), 1.0f),
				glm::vec4(rLocalSpaceAABB.CalcMinimumX(), rLocalSpaceAABB.CalcMinimumY(), rLocalSpaceAABB.CalcMinimumZ(), 1.0f),
			};

			for (const glm::vec4& rCurrentCornerPoint : vecOldAABBCornerPoints) // for every corner point
			{
				// transform it into world space
				const glm::vec4 vec4WorldSpaceCornerPoint = mat4Transform * rCurrentCornerPoint;

				// world space AABB half-widths
				const float fHalfWidthXForCurrentCornerPoint = std::abs(vec4WorldSpaceCornerPoint.x - tResult.m_vec3Center.x);
				const float fHalfWidthYForCurrentCornerPoint = std::abs(vec4WorldSpaceCornerPoint.y - tResult.m_vec3Center.y);
				const float fHalfWidthZForCurrentCornerPoint = std::abs(vec4WorldSpaceCornerPoint.z - tResult.m_vec3Center.z);

				tResult.m_vec3Radius.x = std::max(tResult.m_vec3Radius.x, fHalfWidthXForCurrentCornerPoint);
				tResult.m_vec3Radius.y = std::max(tResult.m_vec3Radius.y, fHalfWidthYForCurrentCornerPoint);
				tResult.m_vec3Radius.z = std::max(tResult.m_vec3Radius.z, fHalfWidthZForCurrentCornerPoint);
			}

			return tResult;
		}

		BoundingSphere ConstructBoundingSphereFromVertexData_Iterative(float * pVertices, size_t uiNumberOfVertices)
		{
			assert(!"not yet implemented");
			const size_t uiNumberOfIterations = 8u;

			BoundingSphere tResult;

			return tResult;
		}

		BoundingSphere ConstructBoundingSphereFromVertexData(float * pVertices, size_t uiNumberOfVertices)
		{
			BoundingSphere tRitterSphere = BoundingSphereFromDistantPoints(pVertices, uiNumberOfVertices);


			for (size_t uiCurrentVertex = 0; uiCurrentVertex < uiNumberOfVertices; uiCurrentVertex++)
			{
				glm::vec3 vec3PointToBeEncompassed(pVertices[uiCurrentVertex * 8 + 0], pVertices[uiCurrentVertex * 8 + 1], pVertices[uiCurrentVertex * 8 + 2]);
				ConditionallyUpdateSphereToEncompassPoint(tRitterSphere, vec3PointToBeEncompassed);
			}



			return tRitterSphere;
		}

		BoundingSphere BoundingSphereFromDistantPoints(float * pVertices, size_t uiNumberOfVertices)
		{
			BoundingSphere tResult;

			MostSeperatedPointsIndices tMostSeperatedPoints = MostSeperatedPointsOnAABB(pVertices, uiNumberOfVertices);

			const glm::vec3 vec3MinPoint(pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 0], pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 1], pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 2]);
			const glm::vec3 vec3MaxPoint(pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 0], pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 1], pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 2]);

			tResult.m_vec3Center = vec3MinPoint * 0.5f + vec3MaxPoint * 0.5f;
			tResult.m_fRadius = std::sqrtf(glm::dot(vec3MaxPoint - tResult.m_vec3Center, vec3MaxPoint - tResult.m_vec3Center));

			return tResult;
		}

		void ConditionallyUpdateSphereToEncompassPoint(BoundingSphere & rSphereToBeUpdated, const glm::vec3 & rvec3PointToBeEncompassed)
		{
			const glm::vec3 vec3DistanceBetweenSphereCenterAndPoint = rvec3PointToBeEncompassed - rSphereToBeUpdated.m_vec3Center;
			const float fSquaredDistance = glm::dot(vec3DistanceBetweenSphereCenterAndPoint, vec3DistanceBetweenSphereCenterAndPoint);

			// conditional update
			if (fSquaredDistance > rSphereToBeUpdated.m_fRadius * rSphereToBeUpdated.m_fRadius)		// comparing squared distances because square roots are expensive
			{
				const float fActualDistance = std::sqrtf(fSquaredDistance);
				const float fNewRadius = rSphereToBeUpdated.m_fRadius * 0.5f + fActualDistance * 0.5f;
				const float fSphereCenterAdjustment = (fNewRadius - rSphereToBeUpdated.m_fRadius) / fActualDistance;
				rSphereToBeUpdated.m_fRadius = fNewRadius;
				rSphereToBeUpdated.m_vec3Center += vec3DistanceBetweenSphereCenterAndPoint * fSphereCenterAdjustment;
			}
		}

		BoundingSphere UpdateBoundingSphere(const BoundingSphere & rLocalSpaceBoundingSphere, const glm::vec3 & rTranslation, const glm::vec3 & rScale)
		{
			BoundingSphere tResult;
			tResult.m_vec3Center = rLocalSpaceBoundingSphere.m_vec3Center + rTranslation;

			float fHighestScalingInAxis = rScale.x;
			if (rScale.y > fHighestScalingInAxis)
				fHighestScalingInAxis = rScale.y;
			if (rScale.z > fHighestScalingInAxis)
				fHighestScalingInAxis = rScale.z;

			tResult.m_fRadius = rLocalSpaceBoundingSphere.m_fRadius * fHighestScalingInAxis;

			//const float fSquaredScalingDistance = glm::dot(rScale, rScale); // equivalent to: (x² + y² + z²)
			//const float fSquaredUnitScaling = 3.0f;							// equivalent to: (1² + 1² + 1²)

			//const float fSquaredRadiusScalingFactor = fSquaredScalingDistance / fSquaredUnitScaling;
			//const float fActualRadiusScalingFactor = std::sqrtf(fSquaredRadiusScalingFactor);

			//tResult.m_fRadius = rLocalSpaceBoundingSphere.m_fRadius * fActualRadiusScalingFactor;

			return tResult;
		}

		BoundingSphere ConstructLocalSpaceBoundingSphereForCube(const SceneObject & rCurrentCube)
		{
			assert(rCurrentCube.m_eType == SceneObject::eType::CUBE);

			BoundingSphere tResult;

			tResult.m_vec3Center = glm::vec3(0.0f, 0.0f, 0.0f);
			assert(std::abs(Primitives::Cube::VertexData[0]) == 50.0f); // your cheat is out of date
			const float fSquaredDistanceToFurthestPoint = glm::dot(glm::vec3(50.0f, 50.0f, 50.0f), glm::vec3(50.0f, 50.0f, 50.0f));
			tResult.m_fRadius = std::sqrtf(fSquaredDistanceToFurthestPoint);

			return tResult;
		}

		BoundingSphere ConstructLocalSpaceBoundingSphereForSphere(const SceneObject & rCurrentSphere)
		{
			assert(rCurrentSphere.m_eType == SceneObject::eType::SPHERE);
			BoundingSphere tResult;

			tResult.m_vec3Center = glm::vec3(0.0f, 0.0f, 0.0f);
			tResult.m_fRadius = Primitives::Sphere::SphereDefaultRadius;

			return tResult;
		}

		//////////////////////////////////////////
		// BOUNDING VOLUME HIERARCHY
		//////////////////////////////////////////

		void RecursiveTopDownTree_AABB(BVHTreeNode ** pTree, SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			assert(pTree);
			assert(pSceneObjects);
			assert(uiNumSceneObjects > 0);

			const uint8_t uiNumberOfObjectsPerLeaf = 1u;
			BVHTreeNode* pNewNode = new BVHTreeNode;
			*pTree = pNewNode;

			if (uiNumSceneObjects <= uiNumberOfObjectsPerLeaf) // is a leaf
			{
				assert(uiNumSceneObjects == 1); // needs reconsideration for >1 objects per leaf
				// bounding volumes for single objects is already done, no need to compute that here
				pNewNode->m_uiNumOjbects = static_cast<uint8_t>(uiNumSceneObjects);
				pNewNode->m_pObjects = pSceneObjects;
			}
			else // is a node
			{
				// create AABB bounding volume for the current set of objects
				pNewNode->m_tAABBForNode = CreateAABBForMultipleObjects(pSceneObjects, uiNumSceneObjects);

				// create Bounding Sphere volume for the current set of objects
				//pNewNode->m_tBoundingSphereForNode = CreateBoundingSphereForMultipleObjects(pSceneObjects, uiNumSceneObjects);

				// partition current set into subsets IN PLACE!!!
				size_t uiPartitioningIndex = PartitionSceneObjectsInPlace_AABB(pSceneObjects, uiNumSceneObjects);

				// move on with "left" side
				RecursiveTopDownTree_AABB(&(pNewNode->m_pLeft), pSceneObjects, uiPartitioningIndex);

				// move on with "right" side
				RecursiveTopDownTree_AABB(&(pNewNode->m_pRight), pSceneObjects + uiPartitioningIndex, uiNumSceneObjects - uiPartitioningIndex);
			}
		}

		BVHTreeNode * BottomUpTree_AABB(SceneObject * pSceneObjects, size_t uiNumSceneObjects, Visualization& rVisualization)
		{
			assert(uiNumSceneObjects > 0);

			BVHTreeNode** pTempNodes = new BVHTreeNode*[uiNumSceneObjects]; // careful: these are pointers to pointers

			// creating all leaf nodes: number leaves == number objects
			for (size_t uiCurrentNewLeafNode = 0u; uiCurrentNewLeafNode < uiNumSceneObjects; uiCurrentNewLeafNode++)
			{
				pTempNodes[uiCurrentNewLeafNode] = new BVHTreeNode;	// assigning the adress of the new leaf node to the pointer pointed at by pTempNodes[current]
				pTempNodes[uiCurrentNewLeafNode]->m_uiNumOjbects = 1u;
				pTempNodes[uiCurrentNewLeafNode]->m_pObjects = &pSceneObjects[uiCurrentNewLeafNode];
				pTempNodes[uiCurrentNewLeafNode]->m_tAABBForNode = pTempNodes[uiCurrentNewLeafNode]->m_pObjects->m_tWorldSpaceAABB;
			}

			// for visualization purposes
			int16_t iNumConstructedNodes = 0;

			// merging leaves into nodes until root node is constructed
			while (uiNumSceneObjects > 1) {
				// Pick two volumes to pair together
				size_t uiMergedNodeIndex1 = 0, uiMergedNodeIndex2 = 0;
				FindBottomUpNodesToMerge_AABB(pTempNodes, uiNumSceneObjects, uiMergedNodeIndex1, uiMergedNodeIndex2);

				// Pair them in new parent node
				BVHTreeNode* pParentNode = new BVHTreeNode;
				pParentNode->m_pLeft = pTempNodes[uiMergedNodeIndex1];
				pParentNode->m_pRight = pTempNodes[uiMergedNodeIndex2];
				// construct AABB for that parent node (adaption from orginal code)
				pParentNode->m_tAABBForNode = MergeTwoAABBs(pTempNodes[uiMergedNodeIndex1]->m_tAABBForNode, pTempNodes[uiMergedNodeIndex2]->m_tAABBForNode);

				// for visualization/rendering purposes
				TreeNodeForRendering tNewAABBNodeForRendering;
				tNewAABBNodeForRendering.m_iRenderingOrder = iNumConstructedNodes++;
				tNewAABBNodeForRendering.m_pNodeToBeRendered = pParentNode;
				rVisualization.m_vecTreeAABBsForBottomUpRendering.push_back(tNewAABBNodeForRendering);

				//Updating the current set of nodes accordingly
				size_t uiMinIndex = uiMergedNodeIndex1, uiMaxIndex = uiMergedNodeIndex2;
				if (uiMergedNodeIndex1 > uiMergedNodeIndex2)
				{
					uiMinIndex = uiMergedNodeIndex2;
					uiMaxIndex = uiMergedNodeIndex1;
				}
				pTempNodes[uiMinIndex] = pParentNode;
				pTempNodes[uiMaxIndex] = pTempNodes[uiNumSceneObjects - 1];
				uiNumSceneObjects--;
			}

			BVHTreeNode* pRootNode = pTempNodes[0]; // careful: getting the pointer to root by dereferencing the pointer to pointer
			delete[] pTempNodes;
			return pRootNode;
		}

		void TraverseTreeForDataForTopDownRendering_AABB(BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecAABBsForRendering, int16_t iDepthInTree, int16_t& riTotalTreeDepth)
		{
			assert(pNode);

			

			if (pNode->IsANode()) // if there is a pointer to objects, it is a leaf
			{
				// if it is a node, there was a partitioning step, which means there have to be two children
				assert(pNode->m_pLeft);
				assert(pNode->m_pRight);

				// save relevant data for rendering
				TreeNodeForRendering tNewAABBForRendering;
				tNewAABBForRendering.m_iDepthInTree = iDepthInTree;
				tNewAABBForRendering.m_pNodeToBeRendered = pNode;
				rvecAABBsForRendering.push_back(tNewAABBForRendering);

				riTotalTreeDepth = std::max(riTotalTreeDepth, iDepthInTree);
				iDepthInTree++; // we are now one level deeper

				// traverse left ...
				TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pLeft, rvecAABBsForRendering, iDepthInTree, riTotalTreeDepth);
				// ... then right
				TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pRight, rvecAABBsForRendering, iDepthInTree, riTotalTreeDepth);
			}

			// Note: In this function we care only for nodes, leaves are intentionally left out since we have that data separated
		}

		void TraverseTreeForDataForBottomUpRendering_AABB(BVHTreeNode * pNode, std::vector<TreeNodeForRendering>& rvecAABBsForRendering, int16_t iDepthInTree, int16_t& riTotalTreeDepth)
		{
			/*
				This is going to be ugly.
				We are facing a problem here: For Bottom up construction of a BVH, the order in which AABBs are constructed is not the same as the tree is later being traversed.
				That means, there is no information in the tree, in which order its nodes were constructed.
				Also, i want to keep this information outside of the tree, since it is really only relevant for rendering/visualizing.
				So, in an effort to keep the visualization only data out of the tree, we have to now do something very ugly.
				Order of construction can only be determined during construction. Tree depth of a given node and its associated AABB can only be determined during traversal.
			*/

			assert(pNode);

			

			if (pNode->IsANode()) // if there is a pointer to objects, it is a leaf
			{
				// if it is a node, there was a partitioning step, which means there have to be two children
				assert(pNode->m_pLeft);
				assert(pNode->m_pRight);

				// the ugly part. finding the appropriate rendering object to assign it its true tree depth.
				for (TreeNodeForRendering& rCurrentRenderObject : rvecAABBsForRendering)
				{
					if (pNode == rCurrentRenderObject.m_pNodeToBeRendered)
					{
						rCurrentRenderObject.m_iDepthInTree = iDepthInTree;
						break;
					}
				}

				riTotalTreeDepth = std::max(riTotalTreeDepth, iDepthInTree);
				iDepthInTree++; // we are now one level deeper				

				// traverse left ...
				TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pLeft, rvecAABBsForRendering, iDepthInTree, riTotalTreeDepth);
				// ... then right
				TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pRight, rvecAABBsForRendering, iDepthInTree, riTotalTreeDepth);
			}
		}

		void FindBottomUpNodesToMerge_AABB(BVHTreeNode ** pNode, size_t uiNumNodes, size_t & rNodeIndex1, size_t & rNodeIndex2)
		{
			float fCurrentlySmallestAABBVolume = std::numeric_limits<float>::max();

			// testing every current node...
			for (size_t uiCurrentMergePartnerIndex1 = 0; uiCurrentMergePartnerIndex1 < uiNumNodes; uiCurrentMergePartnerIndex1++)
			{
				// ... against every other node
				for (size_t uiCurrentMergePartnerIndex2 = uiCurrentMergePartnerIndex1 + 1; uiCurrentMergePartnerIndex2 < uiNumNodes; uiCurrentMergePartnerIndex2++)
				{
					//  determine the size of the bounding volume that would result from the two current 
					const AABB& rAABB1 = pNode[uiCurrentMergePartnerIndex1]->m_tAABBForNode;
					const AABB& rAABB2 = pNode[uiCurrentMergePartnerIndex2]->m_tAABBForNode;

					const float fMergedAABBExtentX = std::abs(rAABB1.m_vec3Center.x - rAABB2.m_vec3Center.x) + rAABB1.m_vec3Radius.x + rAABB2.m_vec3Radius.x;
					const float fMergedAABBExtentY = std::abs(rAABB1.m_vec3Center.y - rAABB2.m_vec3Center.y) + rAABB1.m_vec3Radius.y + rAABB2.m_vec3Radius.y;
					const float fMergedAABBExtentZ = std::abs(rAABB1.m_vec3Center.z - rAABB2.m_vec3Center.z) + rAABB1.m_vec3Radius.z + rAABB2.m_vec3Radius.z;

					const float fMergedAABBVolume = fMergedAABBExtentX * fMergedAABBExtentY * fMergedAABBExtentZ;

					// update results conditionally
					if (fMergedAABBVolume < fCurrentlySmallestAABBVolume)
					{
						fCurrentlySmallestAABBVolume = fMergedAABBVolume;
						rNodeIndex1 = uiCurrentMergePartnerIndex1;
						rNodeIndex2 = uiCurrentMergePartnerIndex2;
					}
				}
			}
		}

		AABB MergeTwoAABBs(const AABB & rAABB1, const AABB & rAABB2)
		{
			AABB tResult;

			const float fXMin = std::min(rAABB1.CalcMinimumX(), rAABB2.CalcMinimumX());
			const float fYMin = std::min(rAABB1.CalcMinimumY(), rAABB2.CalcMinimumY());
			const float fZMin = std::min(rAABB1.CalcMinimumZ(), rAABB2.CalcMinimumZ());
			const float fXMax = std::max(rAABB1.CalcMaximumX(), rAABB2.CalcMaximumX());
			const float fYMax = std::max(rAABB1.CalcMaximumY(), rAABB2.CalcMaximumY());
			const float fZMax = std::max(rAABB1.CalcMaximumZ(), rAABB2.CalcMaximumZ());

			// calculating AABB center
			const float fCenterX = fXMin * 0.5f + fXMax * 0.5f;
			const float fCenterY = fYMin * 0.5f + fYMax * 0.5f;
			const float fCenterZ = fZMin * 0.5f + fZMax * 0.5f;
			tResult.m_vec3Center = glm::vec3(fCenterX, fCenterY, fCenterZ);

			// calculating AABB halfwidths
			const float fXTotalExtent = fXMax - fXMin;
			const float fYTotalExtent = fYMax - fYMin;
			const float fZTotalExtent = fZMax - fZMin;
			const float fHalfWidthX = fXTotalExtent * 0.5f;
			const float fHalfWidthY = fYTotalExtent * 0.5f;
			const float fHalfWidthZ = fZTotalExtent * 0.5f;

			// calculating AABB halfwidths
			tResult.m_vec3Radius = glm::vec3(fHalfWidthX, fHalfWidthY, fHalfWidthZ);

			return tResult;
		}

		AABB CreateAABBForMultipleObjects(const SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			AABB tResult;

			// initializing min values to max and vice versa for definitive overwriting for the first vertex
			float fXMin = std::numeric_limits<float>::max();
			float fXMax = std::numeric_limits<float>::lowest();
			float fYMin = std::numeric_limits<float>::max();
			float fYMax = std::numeric_limits<float>::lowest();
			float fZMin = std::numeric_limits<float>::max();
			float fZMax = std::numeric_limits<float>::lowest();

			for (size_t uiCurrentSceneObjectIndex = 0u; uiCurrentSceneObjectIndex < uiNumSceneObjects; uiCurrentSceneObjectIndex++)
			{
				const SceneObject& rCurrentObject = pSceneObjects[uiCurrentSceneObjectIndex];
				assert(glm::length(rCurrentObject.m_tWorldSpaceAABB.m_vec3Radius) > 0.0f);	// make sure AABB of current object has already been constructed

				// get extent of current AABB
				const float fObjAABBXMin = rCurrentObject.m_tWorldSpaceAABB.CalcMinimumX();
				const float fObjAABBYMin = rCurrentObject.m_tWorldSpaceAABB.CalcMinimumY();
				const float fObjAABBZMin = rCurrentObject.m_tWorldSpaceAABB.CalcMinimumZ();
				const float fObjAABBXMax = rCurrentObject.m_tWorldSpaceAABB.CalcMaximumX();
				const float fObjAABBYMax = rCurrentObject.m_tWorldSpaceAABB.CalcMaximumY();
				const float fObjAABBZMax = rCurrentObject.m_tWorldSpaceAABB.CalcMaximumZ();

				// update resulting bounding volume accordingly
				fXMin = std::min<float>(fXMin, fObjAABBXMin);
				fYMin = std::min<float>(fYMin, fObjAABBYMin);
				fZMin = std::min<float>(fZMin, fObjAABBZMin);
				fXMax = std::max<float>(fXMax, fObjAABBXMax);
				fYMax = std::max<float>(fYMax, fObjAABBYMax);
				fZMax = std::max<float>(fZMax, fObjAABBZMax);
			}

			// calculating AABB center
			const float fCenterX = fXMin * 0.5f + fXMax * 0.5f;
			const float fCenterY = fYMin * 0.5f + fYMax * 0.5f;
			const float fCenterZ = fZMin * 0.5f + fZMax * 0.5f;
			tResult.m_vec3Center = glm::vec3(fCenterX, fCenterY, fCenterZ);

			// calculating AABB halfwidths
			const float fXTotalExtent = fXMax - fXMin;
			const float fYTotalExtent = fYMax - fYMin;
			const float fZTotalExtent = fZMax - fZMin;
			const float fHalfWidthX = fXTotalExtent * 0.5f;
			const float fHalfWidthY = fYTotalExtent * 0.5f;
			const float fHalfWidthZ = fZTotalExtent * 0.5f;

			// calculating AABB halfwidths
			tResult.m_vec3Radius = glm::vec3(fHalfWidthX, fHalfWidthY, fHalfWidthZ);

			return tResult;
		}

		BoundingSphere MergeTwoBoundingSpheres(const BoundingSphere & rBoundingSphere1, const BoundingSphere & rBoundingSphere2)
		{
			assert(rBoundingSphere1.m_fRadius > 0.0f);
			assert(rBoundingSphere2.m_fRadius > 0.0f);

			// initiliazing result with first bounding sphere
			BoundingSphere tResult = rBoundingSphere1;

			// we determine the distance vector to encompassed sphere from result sphere
			const glm::vec3 vec3CenterPointsDistance = rBoundingSphere2.m_vec3Center - tResult.m_vec3Center;
			// we construct the normalized direction of the distance ...
			const glm::vec3 vec3NormalizedCenterPointsDirection = glm::normalize(vec3CenterPointsDistance);
			// ...  to then scale it by the encompassed sphere's radius, resulting in a "directed" radius
			const glm::vec3 vec3DirectedEncompassedSphereRadius = vec3NormalizedCenterPointsDirection * rBoundingSphere2.m_fRadius;
			// we construct the point on the encompassed sphere that is the most distant to the current sphere's center
			const glm::vec3 vec3EncompassedSphereMostDistantPoint = rBoundingSphere2.m_vec3Center + vec3DirectedEncompassedSphereRadius;
			// we conditionally update the result sphere to encompass this most distant point
			ConditionallyUpdateSphereToEncompassPoint(tResult, vec3EncompassedSphereMostDistantPoint);

			return tResult;
		}

		void TraverseTreeForDataForTopDownRendering_BoundingSphere(BVHTreeNode * pNode, std::vector<TreeNodeForRendering>& rvecBoundingspheresForRendering, int16_t iDepthInTree, int16_t& riTotalTreeDepth)
		{
			assert(pNode);

			
			

			if (pNode->IsANode()) // if there is a pointer to objects, it is a leaf
			{
				// if it is a node, there was a partitioning step, which means there have to be two children
				assert(pNode->m_pLeft);
				assert(pNode->m_pRight);

				// save relevant data for rendering
				TreeNodeForRendering tNewBoundingSphereForRendering;
				tNewBoundingSphereForRendering.m_iDepthInTree = iDepthInTree;
				tNewBoundingSphereForRendering.m_pNodeToBeRendered = pNode;
				rvecBoundingspheresForRendering.push_back(tNewBoundingSphereForRendering);

				riTotalTreeDepth = std::max(riTotalTreeDepth, iDepthInTree);
				iDepthInTree++; // we are now one level deeper

				// traverse left ...
				TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pLeft, rvecBoundingspheresForRendering, iDepthInTree, riTotalTreeDepth);
				// ... then right
				TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pRight, rvecBoundingspheresForRendering, iDepthInTree, riTotalTreeDepth);
			}

			// Note: In this function we care only for nodes, leaves are intentionally left out since we have that data separated
		}

		void TraverseTreeForDataForBottomUpRendering_BoundingSphere(BVHTreeNode * pNode, std::vector<TreeNodeForRendering>& rvecBoundingSpheresForRendering, int16_t iDepthInTree, int16_t& riDeepestDepthOfNodes)
		{
			/*
				This is going to be ugly.
				We are facing a problem here: For Bottom up construction of a BVH, the order in which AABBs are constructed is not the same as the tree is later being traversed.
				That means, there is no information in the tree, in which order its nodes were constructed.
				Also, i want to keep this information outside of the tree, since it is really only relevant for rendering/visualizing.
				So, in an effort to keep the visualization only data out of the tree, we have to now do something very ugly.
				Order of construction can only be determined during construction. Tree depth of a given node and its associated AABB can only be determined during traversal.
			*/

			assert(pNode);

			if (pNode->IsANode()) // if there is a pointer to objects, it is a leaf
			{
				// if it is a node, there was a partitioning step, which means there have to be two children
				assert(pNode->m_pLeft);
				assert(pNode->m_pRight);

				// the ugly part. finding the appropriate rendering object to assign it its true tree depth.
				for (TreeNodeForRendering& rCurrentRenderObject : rvecBoundingSpheresForRendering)
				{
					if (pNode == rCurrentRenderObject.m_pNodeToBeRendered)
					{
						rCurrentRenderObject.m_iDepthInTree = iDepthInTree;
						break;
					}
				}

				riDeepestDepthOfNodes = std::max(riDeepestDepthOfNodes, iDepthInTree);
				iDepthInTree++; // we are now one level deeper

				// traverse left ...
				TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pLeft, rvecBoundingSpheresForRendering, iDepthInTree, riDeepestDepthOfNodes);
				// ... then right
				TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pRight, rvecBoundingSpheresForRendering, iDepthInTree, riDeepestDepthOfNodes);
			}
		}

		BoundingSphere CreateBoundingSphereForMultipleObjects(const SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			assert(uiNumSceneObjects >= 2u); // if 1 or less, some error occurred
			assert(pSceneObjects[0].m_tWorldSpaceBoundingSphere.m_fRadius > 0.0f);

			// initiliazing result with first object
			BoundingSphere tResult = pSceneObjects[0].m_tWorldSpaceBoundingSphere;

			// for every FOLLOWING object (its bounding sphere specifically) ...
			for (size_t uiCurrentObjectToBeEncompassed = 1u; uiCurrentObjectToBeEncompassed < uiNumSceneObjects; uiCurrentObjectToBeEncompassed++)
			{
				const BoundingSphere& rCurrentOtherBoundingSphere = pSceneObjects[uiCurrentObjectToBeEncompassed].m_tWorldSpaceBoundingSphere;
				assert(rCurrentOtherBoundingSphere.m_fRadius > 0.0f);

				// we determine the distance vector to encompassed sphere from result sphere
				const glm::vec3 vec3CenterPointsDistance = rCurrentOtherBoundingSphere.m_vec3Center - tResult.m_vec3Center;

				// we construct the normalized direction of the distance ...
				const glm::vec3 vec3NormalizedCenterPointsDirection = glm::normalize(vec3CenterPointsDistance);
				// ...  to then scale it by the encompassed sphere's radius, resulting in a "directed" radius
				const glm::vec3 vec3DirectedEncompassedSphereRadius = vec3NormalizedCenterPointsDirection * rCurrentOtherBoundingSphere.m_fRadius;
				// we construct the point on the encompassed sphere that is the most distant to the current sphere's radius
				const glm::vec3 vec3EncompassedSphereMostDistantPoint = rCurrentOtherBoundingSphere.m_vec3Center + vec3DirectedEncompassedSphereRadius;
				// we conditionally update the result sphere to encompass this most distant point
				ConditionallyUpdateSphereToEncompassPoint(tResult, vec3EncompassedSphereMostDistantPoint);
			}

			// after every "other" sphere has been encompassed, the bounding sphere of all given objects is ready

			return tResult;
		}

		size_t PartitionSceneObjectsInPlace_BoundingSphere(SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			assert(pSceneObjects);
			assert(uiNumSceneObjects > 0u);
			/*
				an explanation:
				This function:
				1. decides the splitting axis
				2. then the splitting point
				3. then partitions the given scene objects
			*/

			// 1. Finding the splitting axis
			// 1.1. Finding the axis with the longest extent

			// initializing with values that will definitely be overwritten
			float fXMaxExtent = std::numeric_limits<float>::lowest();
			float fXMinExtent = std::numeric_limits<float>::max();
			float fYMaxExtent = std::numeric_limits<float>::lowest();
			float fYMinExtent = std::numeric_limits<float>::max();
			float fZMaxExtent = std::numeric_limits<float>::lowest();
			float fZMinExtent = std::numeric_limits<float>::max();

			// finding min and max extents for every axis
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				const SceneObject& rCurrentSceneObject = pSceneObjects[uiCurrentSceneObject];

				fXMaxExtent = std::max(fXMaxExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMaximumX());
				fYMaxExtent = std::max(fYMaxExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMaximumY());
				fZMaxExtent = std::max(fZMaxExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMaximumZ());

				fXMinExtent = std::min(fXMinExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMinimumX());
				fYMinExtent = std::min(fYMinExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMinimumY());
				fZMinExtent = std::min(fZMinExtent, rCurrentSceneObject.m_tWorldSpaceBoundingSphere.CalcMinimumZ());
			}

			// Getting the longest extent of the 3 axes
			const float fXTotalExtent = fXMaxExtent - fXMinExtent;
			const float fYTotalExtent = fYMaxExtent - fYMinExtent;
			const float fZTotalExtent = fZMaxExtent - fZMinExtent;

			const int x = 0, y = 1, z = 2;
			int iSplittingAxis = x;

			if (fYTotalExtent > fXTotalExtent && fYTotalExtent > fZTotalExtent)
				iSplittingAxis = y;

			if (fZTotalExtent > fXTotalExtent && fZTotalExtent > fYTotalExtent)
				iSplittingAxis = z;

			// axis now stores the index of the longest axis in the 3 dimensional coordinate vector

			// 2. Finding the splitting point on the axis
			// done by using the object mean (mean of the object centroids)

			float fObjectCentroidsMean = 0.0f;
			const float fPreDivisionFactor = 1.0f / static_cast<float>(uiNumSceneObjects);

			// iterate over all scene objects and determine the mean by accumulating equally weighted coordinates of the splitting axis
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				const glm::vec3& rCurrentSceneObjectCenter = pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceBoundingSphere.m_vec3Center;
				fObjectCentroidsMean += rCurrentSceneObjectCenter[iSplittingAxis] * fPreDivisionFactor;
			}

			// 3. partitioning the scene objects:
			SceneObject* pCopiedArray = new SceneObject[uiNumSceneObjects];

			// two passes: one for determination of bucket sizes, the second for sorting into buckets
			// first pass
			const size_t uiNumBuckets = 2u;
			size_t uiNumElementsPerBucket[uiNumBuckets] = { 0u };
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				// will be true == 1 for right bucket and false == 0 for left bucket throught implicit type conversion
				const size_t uiBucketIndex = (pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceBoundingSphere.m_vec3Center[iSplittingAxis] >= fObjectCentroidsMean);
				uiNumElementsPerBucket[uiBucketIndex]++;
			}

			assert((uiNumElementsPerBucket[0] + uiNumElementsPerBucket[1]) == uiNumSceneObjects);

			// second pass
			size_t uiBucketInsertionIndices[uiNumBuckets];
			uiBucketInsertionIndices[0u] = 0u;
			uiBucketInsertionIndices[1u] = uiNumElementsPerBucket[0u];

			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				// will be true == 1 for right bucket and false == 0 for left bucket throught implicit type conversion
				const size_t uiBucketIndex = (pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceBoundingSphere.m_vec3Center[iSplittingAxis] >= fObjectCentroidsMean);
				const size_t uiInsertionIndex = uiBucketInsertionIndices[uiBucketIndex]++;
				pCopiedArray[uiInsertionIndex] = pSceneObjects[uiCurrentSceneObject];
			}

			memcpy(pSceneObjects, pCopiedArray, uiNumSceneObjects * sizeof(SceneObject));

			const size_t uiPartitioningIndex = uiNumElementsPerBucket[0]; // number of left children = partitioning index

			return uiPartitioningIndex;
		}

		size_t PartitionSceneObjectsInPlace_AABB(SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			assert(pSceneObjects);
			assert(uiNumSceneObjects > 0u);
			/*
				an explanation:
				This function:
				1. decides the splitting axis
				2. then the splitting point
				3. then partitions the given scene objects
			*/

			// 1. Finding the splitting axis
			// 1.1. Finding the axis with the longest extent

			// initializing with values that will definitely be overwritten
			float fXMaxExtent = std::numeric_limits<float>::lowest();
			float fXMinExtent = std::numeric_limits<float>::max();
			float fYMaxExtent = std::numeric_limits<float>::lowest();
			float fYMinExtent = std::numeric_limits<float>::max();
			float fZMaxExtent = std::numeric_limits<float>::lowest();
			float fZMinExtent = std::numeric_limits<float>::max();

			// finding min and max extents for every axis
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				const SceneObject& rCurrentSceneObject = pSceneObjects[uiCurrentSceneObject];

				fXMaxExtent = std::max(fXMaxExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMaximumX());
				fYMaxExtent = std::max(fYMaxExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMaximumY());
				fZMaxExtent = std::max(fZMaxExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMaximumZ());

				fXMinExtent = std::min(fXMinExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMinimumX());
				fYMinExtent = std::min(fYMinExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMinimumY());
				fZMinExtent = std::min(fZMinExtent, rCurrentSceneObject.m_tWorldSpaceAABB.CalcMinimumZ());
			}

			// Getting the longest extent of the 3 axes
			const float fXTotalExtent = fXMaxExtent - fXMinExtent;
			const float fYTotalExtent = fYMaxExtent - fYMinExtent;
			const float fZTotalExtent = fZMaxExtent - fZMinExtent;

			const int x = 0, y = 1, z = 2;
			int iSplittingAxis = x;

			if (fYTotalExtent > fXTotalExtent && fYTotalExtent > fZTotalExtent)
				iSplittingAxis = y;

			if (fZTotalExtent > fXTotalExtent && fZTotalExtent > fYTotalExtent)
				iSplittingAxis = z;

			// axis now stores the index of the longest axis in the 3 dimensional coordinate vector

			// 2. Finding the splitting point on the axis
			// done by using the object mean (mean of the object centroids)

			float fObjectCentroidsMean = 0.0f;
			const float fPreDivisionFactor = 1.0f / static_cast<float>(uiNumSceneObjects);

			// iterate over all scene objects and determine the mean by accumulating equally weighted coordinates of the splitting axis
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				const glm::vec3& rCurrentSceneObjectCenter = pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceAABB.m_vec3Center;
				fObjectCentroidsMean += rCurrentSceneObjectCenter[iSplittingAxis] * fPreDivisionFactor;
			}

			// 3. partitioning the scene objects:
			SceneObject* pCopiedArray = new SceneObject[uiNumSceneObjects];

			// two passes: one for determination of bucket sizes, the second for sorting into buckets
			// first pass
			const size_t uiNumBuckets = 2u;
			size_t uiNumElementsPerBucket[uiNumBuckets] = { 0u };
			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				// will be true == 1 for right bucket and false == 0 for left bucket throught implicit type conversion
				const size_t uiBucketIndex = (pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceAABB.m_vec3Center[iSplittingAxis] >= fObjectCentroidsMean);
				uiNumElementsPerBucket[uiBucketIndex]++;
			}

			assert((uiNumElementsPerBucket[0] + uiNumElementsPerBucket[1]) == uiNumSceneObjects);

			// second pass
			size_t uiBucketInsertionIndices[uiNumBuckets];
			uiBucketInsertionIndices[0u] = 0u;
			uiBucketInsertionIndices[1u] = uiNumElementsPerBucket[0u];

			for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
			{
				// will be true == 1 for right bucket and false == 0 for left bucket throught implicit type conversion
				const size_t uiBucketIndex = (pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceAABB.m_vec3Center[iSplittingAxis] >= fObjectCentroidsMean);
				const size_t uiInsertionIndex = uiBucketInsertionIndices[uiBucketIndex]++;
				pCopiedArray[uiInsertionIndex] = pSceneObjects[uiCurrentSceneObject];
			}

			memcpy(pSceneObjects, pCopiedArray, uiNumSceneObjects * sizeof(SceneObject));

			const size_t uiPartitioningIndex = uiNumElementsPerBucket[0]; // number of left children = partitioning index

			return uiPartitioningIndex;
		}

		void RecursiveTopDownTree_BoundingSphere(BVHTreeNode ** pNode, SceneObject * pSceneObjects, size_t uiNumSceneObjects)
		{
			assert(pNode);
			assert(pSceneObjects);
			assert(uiNumSceneObjects > 0);

			const uint8_t uiNumberOfObjectsPerLeaf = 1u;
			BVHTreeNode* pNewNode = new BVHTreeNode;
			*pNode = pNewNode;

			if (uiNumSceneObjects <= uiNumberOfObjectsPerLeaf) // is a leaf
			{
				assert(uiNumSceneObjects == 1); // needs reconsideration for >1 objects per leaf
				// bounding volumes for single objects is already done, no need to compute that here
				pNewNode->m_uiNumOjbects = static_cast<uint8_t>(uiNumSceneObjects);
				pNewNode->m_pObjects = pSceneObjects;
			}
			else // is a node
			{
				// create Bounding Sphere volume for the current set of objects
				pNewNode->m_tBoundingSphereForNode = CreateBoundingSphereForMultipleObjects(pSceneObjects, uiNumSceneObjects);

				// partition current set into subsets IN PLACE!!!
				size_t uiPartitioningIndex = PartitionSceneObjectsInPlace_BoundingSphere(pSceneObjects, uiNumSceneObjects);

				// move on with "left" side
				RecursiveTopDownTree_BoundingSphere(&(pNewNode->m_pLeft), pSceneObjects, uiPartitioningIndex);

				// move on with "right" side
				RecursiveTopDownTree_BoundingSphere(&(pNewNode->m_pRight), pSceneObjects + uiPartitioningIndex, uiNumSceneObjects - uiPartitioningIndex);
			}
		}

		BVHTreeNode * BottomUpTree_BoundingSphere(SceneObject * pSceneObjects, size_t uiNumSceneObjects, Visualization & rVisualization)
		{
			assert(uiNumSceneObjects > 0);

			BVHTreeNode** pTempNodes = new BVHTreeNode*[uiNumSceneObjects]; // careful: these are pointers to pointers

			// creating all leaf nodes: number leaves == number objects
			for (size_t uiCurrentNewLeafNode = 0u; uiCurrentNewLeafNode < uiNumSceneObjects; uiCurrentNewLeafNode++)
			{
				pTempNodes[uiCurrentNewLeafNode] = new BVHTreeNode;	// assigning the adress of the new leaf node to the pointer pointed at by pTempNodes[current]
				pTempNodes[uiCurrentNewLeafNode]->m_uiNumOjbects = 1u;
				pTempNodes[uiCurrentNewLeafNode]->m_pObjects = &pSceneObjects[uiCurrentNewLeafNode];
				pTempNodes[uiCurrentNewLeafNode]->m_tBoundingSphereForNode = pTempNodes[uiCurrentNewLeafNode]->m_pObjects->m_tWorldSpaceBoundingSphere;
			}

			// for visualization purposes
			int16_t iNumConstructedNodes = 0;

			// merging leaves into nodes until root node is constructed
			while (uiNumSceneObjects > 1) {
				// Pick two volumes to pair together
				size_t uiMergedNodeIndex1 = 0, uiMergedNodeIndex2 = 0;
				FindBottomUpNodesToMerge_BoundingSphere(pTempNodes, uiNumSceneObjects, uiMergedNodeIndex1, uiMergedNodeIndex2);

				// Pair them in new parent node
				BVHTreeNode* pParentNode = new BVHTreeNode;
				pParentNode->m_pLeft = pTempNodes[uiMergedNodeIndex1];
				pParentNode->m_pRight = pTempNodes[uiMergedNodeIndex2];
				// construct AABB for that parent node (adaption from orginal code)
				pParentNode->m_tBoundingSphereForNode = MergeTwoBoundingSpheres(pTempNodes[uiMergedNodeIndex1]->m_tBoundingSphereForNode, pTempNodes[uiMergedNodeIndex2]->m_tBoundingSphereForNode);

				// for visualization/rendering purposes
				TreeNodeForRendering tNewBoundingSphereNodeForRendering;
				tNewBoundingSphereNodeForRendering.m_iRenderingOrder = iNumConstructedNodes++;
				tNewBoundingSphereNodeForRendering.m_pNodeToBeRendered = pParentNode;
				rVisualization.m_vecTreeBoundingSpheresForBottomUpRendering.push_back(tNewBoundingSphereNodeForRendering);

				//Updating the current set of nodes accordingly
				size_t uiMinIndex = uiMergedNodeIndex1, uiMaxIndex = uiMergedNodeIndex2;
				if (uiMergedNodeIndex1 > uiMergedNodeIndex2)
				{
					uiMinIndex = uiMergedNodeIndex2;
					uiMaxIndex = uiMergedNodeIndex1;
				}
				pTempNodes[uiMinIndex] = pParentNode;
				pTempNodes[uiMaxIndex] = pTempNodes[uiNumSceneObjects - 1];
				uiNumSceneObjects--;
			}

			BVHTreeNode* pRootNode = pTempNodes[0]; // careful: getting the pointer to root by dereferencing the pointer to pointer
			delete[] pTempNodes;
			return pRootNode;
		}

		void FindBottomUpNodesToMerge_BoundingSphere(BVHTreeNode ** pNode, size_t uiNumNodes, size_t & rNodeIndex1, size_t & rNodeIndex2)
		{
			float fCurrentlySmallestBoundingSphereDiameter = std::numeric_limits<float>::max();

			// testing every current node...
			for (size_t uiCurrentMergePartnerIndex1 = 0; uiCurrentMergePartnerIndex1 < uiNumNodes; uiCurrentMergePartnerIndex1++)
			{
				// ... against every other node
				for (size_t uiCurrentMergePartnerIndex2 = uiCurrentMergePartnerIndex1 + 1; uiCurrentMergePartnerIndex2 < uiNumNodes; uiCurrentMergePartnerIndex2++)
				{
					//  determine the size of the bounding volume that would result from the two current 
					const BoundingSphere& rBoundingSphere1 = pNode[uiCurrentMergePartnerIndex1]->m_tBoundingSphereForNode;
					const BoundingSphere& rBoundingSphere2 = pNode[uiCurrentMergePartnerIndex2]->m_tBoundingSphereForNode;

					// we determine the distance vector to sphere2 from sphere1
					const glm::vec3 vec3CenterPointsDistance = rBoundingSphere2.m_vec3Center - rBoundingSphere1.m_vec3Center;
					// we construct the normalized direction of the distance ...
					const glm::vec3 vec3NormalizedCenterPointsDirection = glm::normalize(vec3CenterPointsDistance);
					// ... to then scale it by the second sphere's radius, resulting in a "directed radius"
					const glm::vec3 vec3DirectedSecondSphereRadius = vec3NormalizedCenterPointsDirection * rBoundingSphere2.m_fRadius;
					// ... we also scale its inverse with the first sphere's radius, resulting in a "radius" directed in the opposite direction
					const glm::vec3 vec3DirectedFirstSphereRadius = -vec3NormalizedCenterPointsDirection * rBoundingSphere1.m_fRadius;

					// we construct the point on the second sphere that is the most distant to the first sphere's center
					const glm::vec3 vec3SecondSphereMostDistantPoint = rBoundingSphere2.m_vec3Center + vec3DirectedSecondSphereRadius;
					// we construct the point on the first sphere that is the most distant to the second sphere's center
					const glm::vec3 vec3FirstSphereMostDistantPoint = rBoundingSphere1.m_vec3Center + vec3DirectedFirstSphereRadius;

					// having constructed the two most distant points, we can now simply determine their distance
					const glm::vec3 vec3DistanceBetweenMostDistantPoints = vec3SecondSphereMostDistantPoint - vec3FirstSphereMostDistantPoint;
					// which effectively is the diameter of the bounding sphere that would encompass both spheres.
					const float fMergedBoundingSphereDiameter = glm::length(vec3DistanceBetweenMostDistantPoints);

					// update results conditionally
					if (fMergedBoundingSphereDiameter < fCurrentlySmallestBoundingSphereDiameter)
					{
						fCurrentlySmallestBoundingSphereDiameter = fMergedBoundingSphereDiameter;
						rNodeIndex1 = uiCurrentMergePartnerIndex1;
						rNodeIndex2 = uiCurrentMergePartnerIndex2;
					}
				}
			}
		}

		//////////////////////////////////////////
		// RAY CASTING
		//////////////////////////////////////////

		RayCastIntersectionResult RecursiveRayCastIntoBVHTree(const BVHTreeNode * pNode, const Ray & rCastedRay)
		{
			assert(pNode);

			RayCastIntersectionResult tResultForNodeAndAllItsChilren;

			if (pNode->IsANode())
			{
				float fIntersectionDistanceMin;
				glm::vec3 vec3PointOfIntersection;
				if (IntersectRayAABB(rCastedRay, pNode->m_tAABBForNode, fIntersectionDistanceMin, vec3PointOfIntersection))
				{

					if (pNode->m_pLeft)
					{
						RayCastIntersectionResult tResultLeftChild = RecursiveRayCastIntoBVHTree(pNode->m_pLeft, rCastedRay);
						//if (tResultLeftChild.m_fIntersectionDistance < tResultForNodeAndAllItsChilren.m_fIntersectionDistance) // this would always be true, because default intersection distance is FLT_MAX
						tResultForNodeAndAllItsChilren = tResultLeftChild;
					}

					if (pNode->m_pRight)
					{
						RayCastIntersectionResult tResultRightchild = RecursiveRayCastIntoBVHTree(pNode->m_pRight, rCastedRay);
						if (tResultRightchild.m_fIntersectionDistance < tResultForNodeAndAllItsChilren.m_fIntersectionDistance)
							tResultForNodeAndAllItsChilren = tResultRightchild;
					}
				}
			}
			else // is a leaf
			{
				assert(pNode->m_uiNumOjbects > 0u);
				assert(pNode->m_pObjects);

				/*
					Checking every object in the current leaf.
					This is needed because:
					1: There could be more that one object in the leaf
					2: The first object to be tested might not be the closest to the ray origin, i.e. the first object hit by the ray
				*/
				for (uint8_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < pNode->m_uiNumOjbects; uiCurrentSceneObject++)
				{
					float fIntersectionDistanceForCurrentAABB;
					glm::vec3 vec3CurrentIntersectionPoint;
					if (IntersectRayAABB(rCastedRay, pNode->m_pObjects[uiCurrentSceneObject].m_tWorldSpaceAABB, fIntersectionDistanceForCurrentAABB, vec3CurrentIntersectionPoint))
					{
						// if an intersection occured with the current object's AABB...
						// ... and the distance to the intersection point is shorter than the previously shortest intersection distance
						if (fIntersectionDistanceForCurrentAABB < tResultForNodeAndAllItsChilren.m_fIntersectionDistance)
						{
							// we update the current results
							tResultForNodeAndAllItsChilren.m_fIntersectionDistance = fIntersectionDistanceForCurrentAABB;
							tResultForNodeAndAllItsChilren.m_vec3PointOfIntersection = vec3CurrentIntersectionPoint;
							tResultForNodeAndAllItsChilren.m_pFirstIntersectedSceneObject = pNode->m_pObjects + uiCurrentSceneObject;
						}
					}
				}

				// if, at the end of all intersection tests, no object AABB was hit, the result is still defaulted (which means its intersection distance = FLT_MAX)
			}

			return tResultForNodeAndAllItsChilren;
		}

		int IntersectRayAABB(const Ray & rIntersectingRay, const AABB& rAABB, float & rfIntersectionDistanceMin, glm::vec3 rvec3IntersectionPoint)
		{
			// assert that the direction vector of the ray is normalized. relevant for: see end of function
			assert(rIntersectingRay.m_vec3Direction.length() == glm::normalize(rIntersectingRay.m_vec3Direction).length());

			rfIntersectionDistanceMin = std::numeric_limits<float>::lowest();
			float fIntersectionDistanceMax = std::numeric_limits<float>::max();

			// for all three slabs of the given AABB
			for (int iCurrentSlab = 0u; iCurrentSlab < 3u; iCurrentSlab++)
			{
				if (std::abs(rIntersectingRay.m_vec3Direction[iCurrentSlab]) < std::numeric_limits<float>::epsilon()) // this tests if the ray is parallel to the current slab
				{
					if (rIntersectingRay.m_vec3Origin[iCurrentSlab] < rAABB.CalcMinimumForAxis(iCurrentSlab) || rIntersectingRay.m_vec3Origin[iCurrentSlab] > rAABB.CalcMaximumForAxis(iCurrentSlab))
						return 0; // exit if ray origin is not witin slab -> no chance of intersection
				}
				else
				{
					const float fPredivisonFactor = 1.0f / rIntersectingRay.m_vec3Direction[iCurrentSlab];
					float fIntersectionDistance1 = (rAABB.CalcMinimumForAxis(iCurrentSlab) - rIntersectingRay.m_vec3Origin[iCurrentSlab]) * fPredivisonFactor;
					float fIntersectionDistance2 = (rAABB.CalcMaximumForAxis(iCurrentSlab) - rIntersectingRay.m_vec3Origin[iCurrentSlab]) * fPredivisonFactor;
					// make sure intersection distance 1 is the intersection distance with the near plane of the current slab
					if (fIntersectionDistance1 > fIntersectionDistance2)
						std::swap(fIntersectionDistance1, fIntersectionDistance2);
					// Compute the intersection of slab intersection intervals
					if (fIntersectionDistance1 > rfIntersectionDistanceMin) rfIntersectionDistanceMin = fIntersectionDistance1;
					if (fIntersectionDistance2 < fIntersectionDistanceMax) fIntersectionDistanceMax = fIntersectionDistance2;
					// Exit with no collision as soon as slab intersection becomes empty
					if (rfIntersectionDistanceMin > fIntersectionDistanceMax)
						return 0;
				}
			}

			// Ray intersects all 3 slabs
			rvec3IntersectionPoint = rIntersectingRay.m_vec3Origin + rIntersectingRay.m_vec3Direction * rfIntersectionDistanceMin;
			return 1;
		}
	}
}