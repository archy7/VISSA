#ifndef CAMERA_H
#define CAMERA_H

/*
	von learnopengl.com übernommen und modifiziert
*/

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:

	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	const glm::vec3 WorldUp;	//
	const glm::vec3 WorldFront; // reference vector for quaternion rotation
	// Eular Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// Default camera values
	static const float fYawDefault;
	static const float fPitchDefault;
	static const float fSpeedDefault;
	static const float fSensitivityDefault;
	static const float fZoomDefault;

	// Settings
	bool m_bFreeCameraOn;

	// Constructor with vectors
	explicit Camera(glm::vec3 position/* = glm::vec3(0.0f, 0.0f, 0.0f)*/, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = fYawDefault, float pitch = fPitchDefault);

	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset);

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	inline glm::mat4 GetViewMatrix() const
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Returns the current Position of the camera
	glm::vec3 GetCurrentPosition() const
	{
		return Position;
	}

	// Manually sets the camera to a specific position. Used for tracking shots.
	void SetToPosition(const glm::vec3& vec3NewPosition)
	{
		Position = vec3NewPosition;
	}

	// Returns a quaternion representing the rotation from the direction vector "looking foward" in the world (0, 0, -1)
	const glm::quat GetRotationQuatFromWorldFront() const;

	// Sets a new direction for the camera to face.
	void SetNewFrontVector(glm::vec3 vec3NewFrontVector);

	// Updates internals after camera was rotated by an outside force (like a camera ride)
	void UpdateInternalsFromFrontVector();

	// Returns the world front vector (0,0,-1)
	const glm::vec3& GetWorldFrontVector() const
	{
		return WorldFront;
	}

	const glm::vec3& GetFrontVector() const
	{
		return Front;
	}

	// Toggles CameraLock. A locked camera cannot move freely. (freely here means user controlled)
	void ToggleCameraLock()
	{
		m_bFreeCameraOn = !m_bFreeCameraOn;
	}


private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors();
};
#endif