#pragma once

#include "generalGL.h"

struct GLFWWindowAndOjbectTuple;
class Camera;
class GUI;

struct Window {
	Window();
	~Window();

	struct MousePositionInWindow {
		float m_fXPosition = 0.0f;
		float m_fYPosition = 0.0f;
	};

	GLFWwindow* m_pGLFWwindow;
	int32_t m_iWindowWidth, m_iWindowHeight;
	float m_fLastXOfMouse, m_fLastYOfMouse;
	bool m_bFirstMouse;
private:
	int m_iMouseShowingStack;
public:
	bool m_bIsInitialized;

	void InitWindow();
	void ReInitializationSequence();
	void CleanUpSequence();
	void RecreateWindow();
	int WindowShouldClose();
	void IncreasMouseShowingStack();
	void DecreasMouseShowingStack();
	void ChangeMouseShowingStack(int iChange);
	void SetHardCaptureMouse(bool bIsCaptureNow);
	bool IsMinimized() const;
	bool IsMouseCaptured() const;
	MousePositionInWindow GetCurrentMousePosition() const;

private:
	void UpdateGLFWMouseCaptureBehaviour();
};