#pragma once

#include <string>

#include "generalGL.h"

struct GLFWWindowAndOjbectTuple;
class Camera;
class GUI;

struct Window {
	Window(int32_t iWindowWidth, int32_t iWindowHeight, std::string sWindowName, Window* pContextSharingWindow = nullptr); // TODO: clean this whole context sharing mess up
	~Window();

	struct MousePositionInWindow {
		float m_fXPosition = 0.0f;
		float m_fYPosition = 0.0f;
	};

	GLFWwindow* m_pGLFWwindow;
	Window* m_pContextSharingWindow;
	int32_t m_iWindowWidth, m_iWindowHeight;
	std::string m_sWindowName;
	float m_fLastXOfMouse, m_fLastYOfMouse;
	bool m_bFirstMouse;
//private:
	int m_iMouseShowingStack;
public:
	bool m_bIsInitialized;

	//void InitWindow();
	void SetAsCurrentContext();
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