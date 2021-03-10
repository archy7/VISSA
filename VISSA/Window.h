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
	bool m_bFirstMouse;
	bool m_bMouseCaptured;
	bool m_bIsInitialized;

	void InitWindow();
	void ReInitializationSequence();
	void CleanUpSequence();
	void RecreateWindow();
	int WindowShouldClose();
	void SetMouseCaptured(bool bIsCaptured);
};