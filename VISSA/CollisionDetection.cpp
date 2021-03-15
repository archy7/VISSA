#include "CollisionDetection.h"

#include <limits>
#include <algorithm>

#include "Scene.h"
#include "GeometricPrimitiveData.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace CollisionDetection;	// ok here since we are in a translation unit devoted to the CollisionDetection namespace

void CollisionDetection::ConstructBoundingVolumesForScene(Scene& rScene)
{
	for (Scene::SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		if (rCurrentSceneObject.m_eType == Scene::SceneObject::eType::CUBE)
		{
			rCurrentSceneObject.m_LocalSpaceAABB = ConstructAABBFromVertexData(Primitives::Cube::TexturedVertexData, sizeof(Primitives::Cube::TexturedIndexData) / sizeof(GLfloat));
		}
		else
		{
			//ConstructAABBFromVertexData(Primitives::Sphere::TexturedVertexData, sizeof(Primitives::Cube::TexturedIndexData) / sizeof(GLfloat));
		}
	}
}

void CollisionDetection::UpdateBoundingVolumesForScene(Scene & rScene)
{
	for (Scene::SceneObject& rCurrentSceneObject : rScene.m_vecObjects)
	{
		if (rCurrentSceneObject.m_eType == Scene::SceneObject::eType::CUBE)
		{
			const Scene::SceneObject::Transform& rCurrentObjectTransform = rCurrentSceneObject.m_tTransform;

			glm::mat4 mat4Rotation = glm::mat4(1.0f); // identity 						
			mat4Rotation = glm::rotate(mat4Rotation, glm::radians(rCurrentObjectTransform.m_tRotation.m_fAngle), rCurrentObjectTransform.m_tRotation.m_vec3Vector);

			// with the transform matrix done, its time to construct the new AABB based on the current transform
			rCurrentSceneObject.m_WorldSpaceAABB = UpdateAABBFromAABB(rCurrentSceneObject.m_LocalSpaceAABB, mat4Rotation, rCurrentObjectTransform.m_vec3Position, rCurrentObjectTransform.m_vec3Scale);
		}
		else
		{
			//ConstructAABBFromVertexData(Primitives::Sphere::TexturedVertexData, sizeof(Primitives::Cube::TexturedIndexData) / sizeof(GLfloat));
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
