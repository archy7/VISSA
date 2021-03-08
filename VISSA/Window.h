#pragma once

#include "generalGL.h"

struct GLFWWindowAndOjbectTuple;
class Camera;
class GUI;

struct Window {
	Window();
	~Window();

	GLFWwindow* m_pGLFWwindow;
	int32_t m_iWindowWidth, m_iWindowHeight;
	float m_fLastXOfMouse, m_fLastYOfMouse;
	int m_iDiscreteKeysStates[GLFW_KEY_LAST];
	bool m_bFirstMouse;
	bool m_bMouseCaptured;
	bool m_bIsInitialized;

	void InitWindow();
	void ReInitializationSequence();
	void CleanUpSequence();
	void RecreateWindow();
	int WindowShouldClose();
	void SetMouseCaptured(bool bIsCaptured);
	bool IsDiscreteKeyReady(int iKey);

	// Callbacks
	static void ResizeWindowCallback(GLFWwindow* pWindow, int fNewWidth, int fNewHeight);
	static void MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset);
	static void MouseMoveCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition);
	static void MouseClickCallBack(GLFWwindow* pWindow, int iButton, int iAction, int iModifiers);
	static void GLFWErrorCallBack(int iError, const char* sDescription);


	// Globals
	static void ProcessKeyboardInput(GLFWwindow* pWindow, float fDeltaTime);
	static Window* GetWindowForGLFWwindow(GLFWwindow* pGivenWindow);
	static Camera* GetCameraForGLFWwindow(GLFWwindow* pGivenWindow);
	static GUI* GetGUIForGLFWwindow(GLFWwindow* pGivenWindow);

	static GLFWWindowAndOjbectTuple GLFWwindowLookUpTuple;
};

struct GLFWWindowAndOjbectTuple {
	GLFWwindow* m_pGLFWWindow = nullptr;
	Window* m_pAssociatedWindowObject = nullptr;
	Camera* m_pAssociatedCamera = nullptr;
	GUI* m_pAssociatedGUI = nullptr;
};