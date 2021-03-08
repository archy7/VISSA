#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

struct Window;

class GUI {
public:
	GUI();

	void InitForWindow(Window& rWindow);
	void ToggleMenuState();
	bool IsMenuActive() const;

	void Render(Window& rWindow);
	void RenderMainMenu(Window& rWindow);
	void RenderSimControlPanel(Window& rWindow);
	void ConditionallyRenderQuitConfirmation(Window& rWindow);
	void ConditionallyRenderVisualizationSelectionMenu(Window& rWindow);
	void ConditionallyRenderOptions(Window& rWindow);
private:

	bool m_bShowMainMenu;
	bool m_bShowSimulationControlPanel;
};