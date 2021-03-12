#include "CollisionDetection.h"

#include <limits>
#include <algorithm>

using namespace CollisionDetection;	// ok here since we are in a translation unit devoted to the CollisionDetection namespace

AABB CollisionDetection::MakeAABBFromVertexData(float * pVertices, size_t uiNumberOfVertices)
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
