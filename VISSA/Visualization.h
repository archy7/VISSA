#pragma once

#include <glad/glad.h>

#include "SceneObject.h"
#include "CollisionDetection.h"
#include "Shader.h"
#include "GUI.h"
#include "Camera.h"

class Engine;
struct Window;
class Renderer;

/*
	The base class for all visualizations.
	todo: more documentation
*/
class Visualization {
public:
	Visualization(Window* pMainWindow);
	virtual ~Visualization();

protected:
	/*
		Base Members
	*/
	Window* m_pMainWindow; // required and must never be null.
	float m_fDeltaTime;	// Updated in Update(...);
public:
	/*
		Base Member Functions
	*/
	virtual void Load() = 0;
	virtual void Render() = 0;
	virtual void Update(float fDeltaTime);
	
	// callbacks
	virtual void MouseMoveCallback(GLFWwindow* pWindow, double dXPosition, double dYPosition) = 0;
	virtual void MouseClickCallback(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers) = 0;
	virtual void WindowResizeCallBack(GLFWwindow* pWindow, int iNewWidth, int iNewHeight) = 0;
	virtual void ProcessKeyboardInput() = 0;
};