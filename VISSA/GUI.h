#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

class Engine;
struct Window;
struct SceneObject;

/*
	TODO: there is a strong point to make to get rid of this class and turn it into a namespace. needs consideration
			as of right now, this class is more like a small extra step for rendering the softwares main menu.
*/
class GUI {
public:
	GUI();

	static void InitForWindow(Window& rWindow);
	void ShowMenu(bool bShowMenu);
	bool IsMenuActive() const;
	bool IsMouseCaptured() const;
	void SetCaptureMouse(bool bIsCapturedNow);

	void Render(Engine& rEngine);
	void RenderMainMenu(Engine& rEngine);
	void ConditionallyRenderQuitConfirmation(Engine& rEngine);
	void ConditionallyRenderVisualizationSelectionMenu(Engine& rEngine);
	void ConditionallyRenderOptions(Engine& rEngine);

	static void HelpMarker(const char* sText);
private:
	bool m_bShowMainMenu;
	bool m_bCaptureMouse;
};