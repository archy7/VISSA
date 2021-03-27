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
	bool IsMenuActive() const;
	bool IsMouseCaptured() const;
	void SetCaptureMouse(bool bIsCapturedNow);

	void Render(Engine& rEngine);
	void RenderMainMenu(Engine& rEngine);
	void RenderSimControlPanel(Engine& rEngine);
	void RenderSimOptions(Engine& rEngine);
	void ConditionallyRenderQuitConfirmation(Engine& rEngine);
	void ConditionallyRenderVisualizationSelectionMenu(Engine& rEngine);
	void ConditionallyRenderOptions(Engine& rEngine);
private:

	bool m_bShowMainMenu;
	bool m_bShowSimulationControlPanel;
	bool m_bShowSimulationOptions;
	bool m_bCaptureMouse;
};