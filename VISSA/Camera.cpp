#include "Camera.h"

#include <glad/glad.h>

#include <iostream>

const float Camera::fYawDefault = -90.0f;
const float Camera::fPitchDefault = 0.0f;
const float Camera::fSpeedDefault = 500.0f;
const float Camera::fSensitivityDefault = 0.1f;
const float Camera::fZoomDefault = 45.0f;

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
	Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
	MovementSpeed(fSpeedDefault), 
	MouseSensitivity(fSensitivityDefault), 
	Zoom(fZoomDefault),
	WorldUp(up),
	WorldFront(glm::vec3(0.0f, 0.0f, -1.0f)),
	Position(position),
	Yaw(yaw),
	Pitch(pitch),
	m_bFreeCameraOn(true)
{
	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) :
	Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
	MovementSpeed(fSpeedDefault), 
	MouseSensitivity(fSensitivityDefault), 
	Zoom(fZoomDefault),
	WorldUp(glm::vec3(upX, upY, upZ)),
	WorldFront(glm::vec3(0.0f, 0.0f, -1.0f)),
	Position(glm::vec3(posX, posY, posZ)),
	Yaw(yaw),
	Pitch(pitch),
	m_bFreeCameraOn(true)
{
	updateCameraVectors();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	if (m_bFreeCameraOn) {
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
			Position += Front * velocity;
		if (direction == BACKWARD)
			Position -= Front * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;
	}
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
	if (m_bFreeCameraOn)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateCameraVectors();
	}
}

void Camera::ProcessMouseScroll(float yoffset)
{
	if (m_bFreeCameraOn)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}
}

const glm::quat Camera::GetRotationQuatFromWorldFront() const
{
	/*
		//Notation

		u = rotation axis vector
		v = reference vector (start of rotation)
		w = end vector (end of rotation)
		F = angle of rotation

		Therefore:
		q = [ u * sin(F/2) + cos(F/2)] OR
		  = [ u.x * sin(F/2) + u.y * sin(F/2) + u.z * sin(F/2) + cos(F/2) ] 

		// The calculation:

		vn = normalize(v);
		wn = normalize(w);

		d = DotProduct(vn, wn)
			->check for parallels
			if(d >= 1) -> parallel same direction -> return unit quaternion
			if(d <= -1) -> parallel in opposite direction -> return 180° rotation

		not parallel:

		F = ArcCos(DotProduct(vn, wn)) / 2;
		u = normalize(CrossProduct(vn,wn));

		-> q = [u*sin(F/2) + cos(F/2)]
		qn = normalize(q);

		return qn;
	*/

	///////////////////////////////////////////////////////////////

	const glm::vec3& vec3StartDirection = glm::normalize(WorldFront);
	const glm::vec3& vec3TargetDirection = glm::normalize(Front);

	const float fDotProduct = glm::dot(vec3StartDirection, vec3TargetDirection);

	if (fDotProduct >= 1.0f) // direction vectors are identical (parallel, same direction)
	{
		// return unit quaternion = zero rotation quaternion
		glm::quat zeroRotationQuat;
		zeroRotationQuat.x = 0.0f;
		zeroRotationQuat.y = 0.0f;
		zeroRotationQuat.z = 0.0f;
		zeroRotationQuat.w = 1.0f;
		return zeroRotationQuat;
	}

	if (fDotProduct <= -1.0f) // direction vectors are opposite (parallel, opposite direction)
	{
		// return 180° rotation
		assert(!"not yet returning proper 180° rotation for opposite direction vectors");
		// return ...;
	}

	const float fAngle = acos(fDotProduct);
	const float fHalfAngle = fAngle / 2;

	const glm::vec3 vec3RotationAxis = glm::cross(vec3TargetDirection, vec3StartDirection);
	const glm::vec3 vec3NormalizedRotationAxis = glm::normalize(vec3RotationAxis);

	glm::quat vec4QuaternionRotationNormalized; // NOT ACTUALLY NORMALIzED
	vec4QuaternionRotationNormalized.x = vec3NormalizedRotationAxis.x * sin(fHalfAngle);
	vec4QuaternionRotationNormalized.y = vec3NormalizedRotationAxis.y * sin(fHalfAngle);
	vec4QuaternionRotationNormalized.z = vec3NormalizedRotationAxis.z * sin(fHalfAngle);
	vec4QuaternionRotationNormalized.w = cos(fHalfAngle);

	return vec4QuaternionRotationNormalized;
}

void Camera::SetNewFrontVector(glm::vec3 vec3NewFrontVector)
{
	// Setting new front vector
	Front = glm::normalize(vec3NewFrontVector);

	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::UpdateInternalsFromFrontVector()
{
	Pitch = asin(-Front.y);
	Yaw = atan2(Front.x, Front.z);
}

void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);

	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}