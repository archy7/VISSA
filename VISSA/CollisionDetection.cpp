#include "CollisionDetection.h"

#include <limits>
#include <algorithm>

#include "Visualization.h"
#include "SceneObject.h"
#include "GeometricPrimitiveData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace CollisionDetection;	// ok here since we are in a translation unit devoted to the CollisionDetection namespace

struct CollisionDetection::MostSeperatedPointsIndices{
	size_t m_uiMinVertexIndex;
	size_t m_uiMaxVertexIndex;
};


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

void CollisionDetection::ConstructBoundingVolumesForScene(Visualization& rScene)
{
	for (SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		if (rCurrentSceneObject.m_eType == SceneObject::eType::CUBE)
		{
			rCurrentSceneObject.m_tLocalSpaceAABB = ConstructAABBFromVertexData(Primitives::Cube::VertexData, sizeof(Primitives::Cube::IndexData) / sizeof(GLfloat));
			//rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructRitterSphere(Primitives::Cube::VertexData, sizeof(Primitives::Cube::IndexData) / sizeof(GLfloat));
			rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructLocalSpaceBoundingSphereForCube(rCurrentSceneObject);

		}
		else if(rCurrentSceneObject.m_eType == SceneObject::eType::SPHERE)
		{
			rCurrentSceneObject.m_tLocalSpaceAABB = ConstructAABBFromVertexData(Primitives::Sphere::VertexData, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
			rCurrentSceneObject.m_tLocalSpaceBoundingSphere = ConstructRitterSphere(Primitives::Sphere::VertexData, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
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
		if (rCurrentSceneObject.m_eType == SceneObject::eType::CUBE)
		{
			const SceneObject::Transform& rCurrentObjectTransform = rCurrentSceneObject.m_tTransform;

			glm::mat4 mat4Rotation = glm::mat4(1.0f); // identity					
			mat4Rotation = glm::rotate(mat4Rotation, glm::radians(rCurrentObjectTransform.m_tRotation.m_fAngle), rCurrentObjectTransform.m_tRotation.m_vec3Vector);

			// construct the new AABB based on the current transform
			rCurrentSceneObject.m_tWorldSpaceAABB = UpdateAABBFromAABB(rCurrentSceneObject.m_tLocalSpaceAABB, mat4Rotation, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
			// updated Bounding Sphere
			rCurrentSceneObject.m_tWorldSpaceBoundingSphere = UpdateBoundingSphere(rCurrentSceneObject.m_tLocalSpaceBoundingSphere, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
		}
		else if (rCurrentSceneObject.m_eType == SceneObject::eType::SPHERE)
		{
			const SceneObject::Transform& rCurrentObjectTransform = rCurrentSceneObject.m_tTransform;

			glm::mat4 mat4Rotation = glm::mat4(1.0f); // identity
			mat4Rotation = glm::rotate(mat4Rotation, glm::radians(rCurrentObjectTransform.m_tRotation.m_fAngle), rCurrentObjectTransform.m_tRotation.m_vec3Vector);

			// construct the new AABB based on the current transform
			rCurrentSceneObject.m_tWorldSpaceAABB = UpdateAABBFromAABB(rCurrentSceneObject.m_tLocalSpaceAABB, mat4Rotation, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
			// updated Bounding Sphere
			rCurrentSceneObject.m_tWorldSpaceBoundingSphere = UpdateBoundingSphere(rCurrentSceneObject.m_tLocalSpaceBoundingSphere, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
		}
		else
		{
			assert(!"nothing here");
		}
	}
}

AABB CollisionDetection::ConstructAABBFromVertexData(float * pVertices, size_t uiNumberOfVertices)
{	
	AABB tResult;

	// initializing min values to max and vice versa for definitive overwriting for the first vertex
	float fXMin = std::numeric_limits<float>::max();
	float fXMax = std::numeric_limits<float>::min();
	float fYMin = std::numeric_limits<float>::max();
	float fYMax = std::numeric_limits<float>::min();
	float fZMin = std::numeric_limits<float>::max();
	float fZMax = std::numeric_limits<float>::min();

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
		maximum and minimum extents along every axis are now stored
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

AABB CollisionDetection::UpdateAABBFromAABB(const AABB& rOldAABB, const glm::mat4& mat4Rotation, const glm::vec3& rTranslation, const glm::vec3& rScale)
{
	assert(glm::length(rOldAABB.m_vec3Radius) > 0.0f);	// make sure old AABB has already been constructed

	AABB tResult;
	
	for (int i = 0; i < 3; i++) // for every axis of the new AABB
	{
		tResult.m_vec3Center[i] = rTranslation[i];	// we assume the translation
		tResult.m_vec3Radius[i] = 0.0f;				// and start with a radius of 0
		for (int j = 0; j < 3; j++)		// for every axis of the old AABB
		{
			tResult.m_vec3Center[i] += mat4Rotation[i][j] * rOldAABB.m_vec3Center[j];				// we adjust the center. This only has an effect, when the old AABBs center was not {0,0,0}, like somewhere in world-space.	When the old AABBs centre point was in 0,0,0 local space, this will do nothing.
			tResult.m_vec3Radius[i] += std::abs(mat4Rotation[i][j]) * rOldAABB.m_vec3Radius[j];		// we continuosly increase the radius, starting at 0. For every axis of the old, rotated old AABB (j), we add its impact to the radius of the axis of the new AABB (i)
		}
		tResult.m_vec3Radius[i] *= rScale[i]; // we scale the current axis of the new AABB
	}

	return tResult;
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

BoundingSphere CollisionDetection::ConstructBoundingSphereFromVertexData(float * pVertices, size_t uiNumberOfVertices)
{
	const size_t uiNumberOfIterations = 8u;

	BoundingSphere tResult;

	return tResult;
}

BoundingSphere CollisionDetection::ConstructRitterSphere(float * pVertices, size_t uiNumberOfVertices)
{
	BoundingSphere tRitterSphere = BoundingSphereFromDistantPoints(pVertices, uiNumberOfVertices);

	
	for (size_t uiCurrentVertex = 0; uiCurrentVertex < uiNumberOfVertices; uiCurrentVertex++)
	{
		glm::vec3 vec3PointToBeEncompassed(pVertices[uiCurrentVertex * 8 + 0], pVertices[uiCurrentVertex * 8 + 1], pVertices[uiCurrentVertex * 8 + 2]);
		UpdateSphereToEncompassPoint(tRitterSphere, vec3PointToBeEncompassed);
	}

	

	return tRitterSphere;
}

MostSeperatedPointsIndices CollisionDetection::MostSeperatedPointsOnAABB(float * pVertices, size_t uiNumberOfVertices)
{
	MostSeperatedPointsIndices tResult;

	size_t uiMinXIndex = 0u, uiMaxXIndex = 0u, uiMinYIndex = 0u, uiMaxYIndex = 0u, uiMinZIndex = 0u, uiMaxZIndex = 0u;
	for (size_t uiCurrentVertexIndex = 1; uiCurrentVertexIndex < uiNumberOfVertices; uiCurrentVertexIndex++)
	{
		// min and max for X: *8 accounts for stride, +0 accounts for x coordinate
		if (pVertices[uiCurrentVertexIndex * 8 + 0] < pVertices[uiMinXIndex * 8 + 0]) uiMinXIndex = uiCurrentVertexIndex;
		if (pVertices[uiCurrentVertexIndex * 8 + 0] > pVertices[uiMaxXIndex * 8 + 0]) uiMaxXIndex = uiCurrentVertexIndex;
		// min and max for Y: *8 accounts for stride, +1 accounts for y coordinate
		if (pVertices[uiCurrentVertexIndex * 8 + 1] < pVertices[uiMinYIndex * 8 + 1]) uiMinYIndex = uiCurrentVertexIndex;
		if (pVertices[uiCurrentVertexIndex * 8 + 1] > pVertices[uiMaxYIndex * 8 + 1]) uiMaxYIndex = uiCurrentVertexIndex;
		// min and max for Y: *8 accounts for stride, +2 accounts for z coordinate
		if (pVertices[uiCurrentVertexIndex * 8 + 2] < pVertices[uiMinZIndex * 8 + 2]) uiMinZIndex = uiCurrentVertexIndex;
		if (pVertices[uiCurrentVertexIndex * 8 + 2] > pVertices[uiMaxZIndex * 8 + 2]) uiMaxZIndex = uiCurrentVertexIndex;
	}

	// creating points for distance calculation
	const glm::vec3 vec3MinXPoint(pVertices[uiMinXIndex * 8 + 0], pVertices[uiMinXIndex * 8 + 1], pVertices[uiMinXIndex * 8 + 2]);
	const glm::vec3 vec3MaxXPoint(pVertices[uiMaxXIndex * 8 + 0], pVertices[uiMaxXIndex * 8 + 1], pVertices[uiMaxXIndex * 8 + 2]);
	const glm::vec3 vec3MinYPoint(pVertices[uiMinYIndex * 8 + 0], pVertices[uiMinYIndex * 8 + 1], pVertices[uiMinYIndex * 8 + 2]);
	const glm::vec3 vec3MaxYPoint(pVertices[uiMaxYIndex * 8 + 0], pVertices[uiMaxYIndex * 8 + 1], pVertices[uiMaxYIndex * 8 + 2]);
	const glm::vec3 vec3MinZPoint(pVertices[uiMinZIndex * 8 + 0], pVertices[uiMinZIndex * 8 + 1], pVertices[uiMinZIndex * 8 + 2]);
	const glm::vec3 vec3MaxZPoint(pVertices[uiMaxZIndex * 8 + 0], pVertices[uiMaxZIndex * 8 + 1], pVertices[uiMaxZIndex * 8 + 2]);

	// calculating the distace
	const glm::vec3 vec3DistanceXPoints = vec3MaxXPoint - vec3MinXPoint;
	const glm::vec3 vec3DistanceYPoints = vec3MaxYPoint - vec3MinYPoint;
	const glm::vec3 vec3DistanceZPoints = vec3MaxZPoint - vec3MinZPoint;

	// computing the squared distance for all pairs
	const float fSquaredDistanceXPoints = glm::dot(vec3DistanceXPoints, vec3DistanceXPoints);
	const float fSquaredDistanceYPoints = glm::dot(vec3DistanceYPoints, vec3DistanceYPoints);
	const float fSquaredDistanceZPoints = glm::dot(vec3DistanceZPoints, vec3DistanceZPoints);

	// references only for readability
	size_t& ruiMinVertexIndex = tResult.m_uiMinVertexIndex;
	size_t& ruiMaxVertexIndex = tResult.m_uiMaxVertexIndex;

	// base assumption: arbitrary axis, here X, is the longest
	ruiMinVertexIndex = uiMinXIndex;
	ruiMaxVertexIndex = uiMaxXIndex;

	// correction into Y axis if those points are farther apart
	if (fSquaredDistanceYPoints > fSquaredDistanceXPoints && fSquaredDistanceYPoints > fSquaredDistanceZPoints)
	{
		ruiMinVertexIndex = uiMinYIndex;
		ruiMaxVertexIndex = uiMaxYIndex;
	}

	// correction into Z axis if those points are farther apart
	if (fSquaredDistanceZPoints > fSquaredDistanceXPoints && fSquaredDistanceZPoints > fSquaredDistanceYPoints)
	{
		ruiMinVertexIndex = uiMinZIndex;
		ruiMaxVertexIndex = uiMaxZIndex;
	}

	return tResult;
}

BoundingSphere CollisionDetection::BoundingSphereFromDistantPoints(float * pVertices, size_t uiNumberOfVertices)
{
	BoundingSphere tResult;

	MostSeperatedPointsIndices tMostSeperatedPoints = MostSeperatedPointsOnAABB(pVertices, uiNumberOfVertices);

	const glm::vec3 vec3MinPoint(pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 0], pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 1], pVertices[tMostSeperatedPoints.m_uiMinVertexIndex * 8 + 2]);
	const glm::vec3 vec3MaxPoint(pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 0], pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 1], pVertices[tMostSeperatedPoints.m_uiMaxVertexIndex * 8 + 2]);

	tResult.m_vec3Center = vec3MinPoint * 0.5f + vec3MaxPoint * 0.5f;
	tResult.m_fRadius = std::sqrtf(glm::dot(vec3MaxPoint - tResult.m_vec3Center, vec3MaxPoint - tResult.m_vec3Center));

	return tResult;
}

void CollisionDetection::UpdateSphereToEncompassPoint(BoundingSphere & rSphereToBeUpdated, const glm::vec3 & rvec3PointToBeEncompassed)
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

BoundingSphere CollisionDetection::ConstructLocalSpaceBoundingSphereForCube(const SceneObject & rCurrentCube)
{
	assert(rCurrentCube.m_eType == SceneObject::eType::CUBE);

	BoundingSphere tResult;

	tResult.m_vec3Center = glm::vec3(0.0f, 0.0f, 0.0f);
	assert(std::abs(Primitives::Cube::VertexData[0]) == 50.0f); // your cheat is out of date
	const float fSquaredDistanceToFurthestPoint = glm::dot(glm::vec3(50.0f, 50.0f, 50.0f), glm::vec3(50.0f, 50.0f, 50.0f));
	tResult.m_fRadius = std::sqrtf(fSquaredDistanceToFurthestPoint);

	return tResult;
}

BoundingSphere CollisionDetection::ConstructLocalSpaceBoundingSphereForSphere(const SceneObject & rCurrentSphere)
{
	assert(rCurrentSphere.m_eType == SceneObject::eType::SPHERE);
	BoundingSphere tResult;

	tResult.m_vec3Center = glm::vec3(0.0f, 0.0f, 0.0f);
	tResult.m_fRadius = Primitives::Sphere::SphereDefaultRadius;

	return tResult;
}

BoundingSphere CollisionDetection::UpdateBoundingSphere(const BoundingSphere & rLocalSpaceBoundingSphere, const glm::vec3 & rTranslation, const glm::vec3 & rScale)
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

	//////////////////////////////////////////////////////////////
	//////////////////////////BVH/////////////////////////////////
	//////////////////////////////////////////////////////////////

CollisionDetection::BoundingVolumeHierarchy CollisionDetection::ConstructBVHForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BoundingVolumeHierarchy tResult;
	tResult.m_pRootNode = new BVHTreeNode;
	rScene.m_vecTreeAABBsForRendering.reserve(rScene.m_vecObjects.size());
	
	// the construction
	TopDownBVTree(&(tResult.m_pRootNode), rScene.m_vecObjects.data(), rScene.m_vecObjects.size());

	// first traversal to gather data for rendering. In theory, it is possible to traverse the tree every frame for BV rendering.
	// But that is terrible, so data is fetched into a linear vector
	TraverseTreeForAABBDataForRendering(tResult.m_pRootNode, rScene.m_vecTreeAABBsForRendering, 0u);

	return tResult;
}

void CollisionDetection::TraverseTreeForAABBDataForRendering(BVHTreeNode* pNode, std::vector<TreeNodeAABBForRendering>& rvecAABBsForRendering, int16_t iTreeDepth)
{
	assert(pNode);

	iTreeDepth++; // we are now one level deeper. depth of root node == 1 here

	if (pNode->IsANode()) // if there is a pointer to objects, it is a leaf
	{
		// if it is a node, there was a partitioning step, which means there have to be two children
		assert(pNode->m_pLeft);
		assert(pNode->m_pRight);

		// save relevant data for rendering
		TreeNodeAABBForRendering tNewAABBForRendering;
		tNewAABBForRendering.m_iTreeDepth = iTreeDepth;
		tNewAABBForRendering.m_tAABBForRendering = pNode->m_tAABBForNode;
		rvecAABBsForRendering.push_back(tNewAABBForRendering);

		// traverse left ...
		TraverseTreeForAABBDataForRendering(pNode->m_pLeft, rvecAABBsForRendering, iTreeDepth);
		// ... then right
		TraverseTreeForAABBDataForRendering(pNode->m_pRight, rvecAABBsForRendering, iTreeDepth);
	}

	// Note: In this function we care for nodes, leaves are intentionally left out
}

void CollisionDetection::TopDownBVTree(BVHTreeNode ** pTree, SceneObject * pSceneObjects, size_t uiNumSceneObjects)
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
		//pNewNode->m_tBoundingSphereForNode = CreateBoundingSphereForMultipleSceneObjects(pSceneObjects, uiNumSceneObjects);

		// partition current set into subsets IN PLACE!!!
		size_t uiPartitioningIndex = PartitionSceneObjectsInPlace(pSceneObjects, uiNumSceneObjects);

		// move on with "left" side
		TopDownBVTree(&(pNewNode->m_pLeft), pSceneObjects, uiPartitioningIndex);

		// move on with "right" side
		TopDownBVTree(&(pNewNode->m_pRight), pSceneObjects + uiPartitioningIndex, uiNumSceneObjects - uiPartitioningIndex);
	}
}

AABB CollisionDetection::CreateAABBForMultipleObjects(const SceneObject * pScenObjects, size_t uiNumSceneObjects)
{
	AABB tResult;

	// initializing min values to max and vice versa for definitive overwriting for the first vertex
	float fXMin = std::numeric_limits<float>::max();
	float fXMax = std::numeric_limits<float>::min();
	float fYMin = std::numeric_limits<float>::max();
	float fYMax = std::numeric_limits<float>::min();
	float fZMin = std::numeric_limits<float>::max();
	float fZMax = std::numeric_limits<float>::min();

	for (size_t uiCurrentSceneObjectIndex = 0u; uiCurrentSceneObjectIndex < uiNumSceneObjects; uiCurrentSceneObjectIndex++)
	{
		const SceneObject& rCurrentObject = pScenObjects[uiCurrentSceneObjectIndex];
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

BoundingSphere CollisionDetection::CreateBoundingSphereForMultipleSceneObjects(const SceneObject * pScenObjects, size_t uiNumSceneObjects)
{
	assert(!"not done yet xD");

	return BoundingSphere();
}

size_t CollisionDetection::PartitionSceneObjectsInPlace(SceneObject * pSceneObjects, size_t uiNumSceneObjects)
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
	float fXMaxExtent = std::numeric_limits<float>::min();
	float fXMinExtent = std::numeric_limits<float>::max();
	float fYMaxExtent = std::numeric_limits<float>::min();
	float fYMinExtent = std::numeric_limits<float>::max();
	float fZMaxExtent = std::numeric_limits<float>::min();
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
	// this is not the cleanest/most efficient implementation for that
	//std::vector<SceneObject*> vecLeftChildren;
	//std::vector<SceneObject*> vecRightChildren;
	//vecLeftChildren.reserve(uiNumSceneObjects);
	//vecRightChildren.reserve(uiNumSceneObjects);

	//for (size_t uiCurrentSceneObject = 0u; uiCurrentSceneObject < uiNumSceneObjects; uiCurrentSceneObject++)
	//{
	//	if (pSceneObjects[uiCurrentSceneObject].m_tWorldSpaceAABB.m_vec3Center[iSplittingAxis] < fObjectCentroidsMean)
	//	{
	//		vecLeftChildren.push_back(pSceneObjects + uiCurrentSceneObject);
	//	}
	//	else // >=
	//	{
	//		vecRightChildren.push_back(pSceneObjects + uiCurrentSceneObject);
	//	}
	//}

	//// the return value
	//size_t uiPartitioningIndex = vecLeftChildren.size();

	//// in place rearrangement
	//memcpy(pSceneObjects, vecLeftChildren.data(), sizeof(SceneObject*) * vecLeftChildren.size());
	//memcpy(pSceneObjects + vecLeftChildren.size(), vecRightChildren.data(), sizeof(SceneObject*) * vecRightChildren.size());

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