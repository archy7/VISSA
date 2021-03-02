#pragma once

#include "generalGL.h"

struct GLFWWindowAndOjbectTuple;
class Camera;

struct Window {
	Window();
	~Window();

	GLFWwindow* m_pGLFWwindow;
	int32_t m_iWindowWidth, m_iWindowHeight;
	float m_fLastXOfMouse, m_fLastYOfMouse;
	bool m_bFirstMouse;
	bool m_bIsInitialized;

	void InitWindow();
	void ReInitializationSequence();
	void CleanUpSequence();
	void RecreateWindow();
	int WindowShouldClose();

	// Callbacks
	static void ProcessInput(GLFWwindow* pWindow, float fDeltaTime);
	static void ResizeWindowCallback(GLFWwindow* pWindow, int fNewWidth, int fNewHeight);
	static void ScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset);
	static void MouseCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition);

	// Globals

	/*
		There are better, more sophisticated ways of solving this, but they are overkill for now.
		This is made on the off chance a second window is SUDDENLY needed.
	*/
	static GLFWWindowAndOjbectTuple GLFWwindowLookUpTable[1];
};

struct GLFWWindowAndOjbectTuple {
	GLFWwindow* m_pGLFWWindow = nullptr;
	Window* m_pAssociatedWindowObject = nullptr;
	Camera* m_pAssociatedCamera = nullptr;
};