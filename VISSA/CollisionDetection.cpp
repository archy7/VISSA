#include "CollisionDetection.h"

#include <limits>
#include <algorithm>

#include "Scene.h"
#include "SceneObject.h"
#include "GeometricPrimitiveData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace CollisionDetection;	// ok here since we are in a translation unit devoted to the CollisionDetection namespace

struct CollisionDetection::MostSeperatedPointsIndices{
	size_t m_uiMinVertexIndex;
	size_t m_uiMaxVertexIndex;
};

void CollisionDetection::ConstructBoundingVolumesForScene(Scene& rScene)
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

void CollisionDetection::UpdateBoundingVolumesForScene(Scene & rScene)
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
