#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "generalGL.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"
#include "GUI.h"
#include "Scene.h"
#include "CollisionDetection.h"

class Engine {
	friend GUI;
public:
	Engine();
	~Engine();

	void MainLoop();
	
	void ProcessKeyboardInput();
	bool IsDiscreteKeyReady(int iKey);

	// Globals
	static Engine* GetGlobalEngine();

	// Callbacks
	static void ResizeWindowCallback(GLFWwindow* pWindow, int fNewWidth, int fNewHeight);
	static void MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset);
	static void MouseMoveCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition);
	static void MouseClickCallBack(GLFWwindow* pWindow, int iButton, int iAction, int iModifiers);
	static void GLFWErrorCallBack(int iError, const char* sDescription);
	
private:
	// Members
	Window m_tWindow;
	Renderer m_tRenderer;
	Camera m_tCamera;
	GUI m_tGUI;
	Scene m_tScene;
	//CollisionDetection::CollisionDetectionSystem m_tCDSystem;
	int m_iDiscreteKeysStates[GLFW_KEY_LAST];
	float m_fDeltaTime, m_fLastFrame, m_fCurrentFrame;

	// Globals
	static Engine* sm_pEngine;

	void InitEngine();
};