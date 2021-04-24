#pragma once

#include <string>

#include "generalGL.h"


class Camera;
class GUI;
struct ImGuiContext;

struct Window {
	Window(int32_t iWindowWidth, int32_t iWindowHeight, std::string sWindowName, GLFWwindow* pContextSharingWindow); // todo: refactor: factory method + private constructor?
	~Window();

	struct MousePositionInWindow {
		float m_fXPosition = 0.0f;
		float m_fYPosition = 0.0f;
	};

	GLFWwindow* m_pGLFWwindow;
	GLFWwindow* m_pContextSharingWindow;
	ImGuiContext* m_pImGuiContext;
	int32_t m_iWindowWidth, m_iWindowHeight;
	std::string m_sWindowName;
	float m_fLastXOfMouse, m_fLastYOfMouse;
	bool m_bFirstMouse;
//private:
	int m_iMouseShowingStack;
public:
	bool m_bIsInitialized;

	//void InitWindow();
	void SetAsCurrentRenderContext();
	void SetAsCurrentGUIContext();
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