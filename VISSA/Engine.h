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
#include "Visualization.h"
#include "CollisionDetection.h"

class Engine {
	friend GUI;		// todo: reconsider this
public:
	Engine();
	~Engine();

	void MainLoop();
	static Window* GetMainWindow();
	
	void ProcessKeyboardInput();
	static bool IsDiscreteKeyReadyForWindow(GLFWwindow* pWindow, int iKey);

	// Globals
	static Engine* GetGlobalEngine();
	static Window* MakeWindowAndSetAsContext(int32_t iWindowWidth, int32_t iWindowHeight, std::string sWindowName); // TODO: remove the last parameter, decision has been made that no GPU resources are shared
	
	// Callbacks
	static void ResizeWindowCallback(GLFWwindow* pWindow, int fNewWidth, int fNewHeight);
	static void MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset);
	static void MouseMoveCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition);
	static void MouseClickCallBack(GLFWwindow* pWindow, int iButton, int iAction, int iModifiers);
	static void GLFWErrorCallBack(int iError, const char* sDescription);
	
private:
	// Members
	Window* m_pMainWindow;
	Renderer m_tRenderer;
	GUI m_tGUI;
	Visualization* m_pVisualization;
	int m_iDiscreteKeysStates[GLFW_KEY_LAST];
	float m_fDeltaTime, m_fLastFrame, m_fCurrentFrame;

	// Globals
	static Engine* sm_pEngine;
	static bool MainWindowCreated(); // for window creation process

	void InitEngine();
	
};