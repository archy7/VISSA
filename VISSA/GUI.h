#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Engine;
struct Window;

class GUI {
public:
	GUI();

	void InitForWindow(Window& rWindow);
	void ShowMenu(bool bShowMenu);
	void ShowObjectPropertiesWindow(bool bShowIt);
	bool IsMenuActive() const;
	bool IsMouseCaptured() const;
	void SetCaptureMouse(bool bIsCapturedNow);
	void SetObjectPropertiesWindowPosition(float fXPosition, float fYPosition);

	void Render(Engine& rEngine);
	void RenderMainMenu(Engine& rEngine);
	void RenderSimControlPanel(Engine& rEngine);
	void RenderSimOptions(Engine& rEngine);
	void RenderObjectPropertiesWindow(Engine& rEngine);
	void RenderObjectCreationWindow(Engine& rEngine);
	void ConditionallyRenderQuitConfirmation(Engine& rEngine);
	void ConditionallyRenderVisualizationSelectionMenu(Engine& rEngine);
	void ConditionallyRenderOptions(Engine& rEngine);
private:
	ImVec2 m_vec2ObjectPropertiesWindowPosition;

	bool m_bShowMainMenu;
	bool m_bShowSimulationControlPanel;
	bool m_bShowSimulationOptions;
	bool m_bShowObjectPropertiesWindow;
	bool m_bShowObjectCreationWindow;
	bool m_bDisplayObjectPropertiesChangesWereMade;
	bool m_bCaptureMouse;
};