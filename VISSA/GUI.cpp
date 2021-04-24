#include "GUI.h"

#include "Window.h"
#include "Engine.h"

#include <stdio.h>

GUI::GUI() :
	m_bShowMainMenu(true)
{

}

void GUI::InitForWindow(Window& rWindow)
{
	assert(rWindow.m_bIsInitialized);

	//rWindow.SetAsCurrentRenderContext();

	IMGUI_CHECKVERSION();

	rWindow.m_pImGuiContext = ImGui::CreateContext();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(rWindow.m_pGLFWwindow, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void GUI::ShowMenu(bool bShowMenu)
{
	m_bShowMainMenu = bShowMenu;
}

bool GUI::IsMenuActive() const
{
	return m_bShowMainMenu;
}

bool GUI::IsMouseCaptured() const
{
	return m_bCaptureMouse;
}

void GUI::SetCaptureMouse(bool bIsCapturedNow)
{
	m_bCaptureMouse = bIsCapturedNow;
}

void GUI::Render(Engine& rEngine)
{
	rEngine.m_pMainWindow->SetAsCurrentRenderContext();

	/*if (m_bShowSimulationControlPanel)
		RenderSimControlPanel(rEngine);*/

	if (m_bShowMainMenu)
		RenderMainMenu(rEngine);

	/*if (m_bShowSimulationOptions)
		RenderSimOptions(rEngine);*/

	/*if (m_bShowObjectPropertiesWindow)
		RenderObjectPropertiesWindow(rEngine);*/

	/*if (m_bShowObjectCreationWindow)
		RenderObjectCreationWindow(rEngine);*/

	/*if (m_bShowHelpWindow)
		RenderHelpWindow(rEngine);*/
}

void GUI::RenderMainMenu(Engine& rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImGui::SetNextWindowPos(main_viewport->WorkPos);
	ImGui::SetNextWindowSize(main_viewport->WorkSize);


	ImGuiWindowFlags backgroundFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	ImGui::Begin("Main Menu Background", NULL, backgroundFlags);

	//ImGui::Text("TRANSPARENT MENU BACK GROUND");

	ImGuiWindowFlags menuFlags = 0;
	ImVec2 mainMenuWindowSize(rEngine.GetMainWindow()->m_iWindowWidth / 2, rEngine.GetMainWindow()->m_iWindowHeight);
	ImVec2 mainMenuWindowPosition(main_viewport->GetCenter().x - rEngine.GetMainWindow()->m_iWindowWidth / 4, main_viewport->WorkPos.y);
	ImGui::SetNextWindowPos(mainMenuWindowPosition);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::BeginChild("Main Menu", mainMenuWindowSize, true, menuFlags);

	ImGui::Text("MENU");

	// these buttons will open another window when pressed, see below
	if (ImGui::Button("VISUALIZATIONS"))
	{
		ImGui::OpenPopup("VISUALIZATIONS");
	}
	//if (ImGui::Button("OPTIONS"))
	//{
	//	ImGui::OpenPopup("OPTIONS");
	//}
	if (ImGui::Button("QUIT"))
	{
		ImGui::OpenPopup("CONFIRM QUIT");
	}

	// Conditions are internally handled by the GUI library and set to true by the above ::OpenPopup() commands
	ConditionallyRenderVisualizationSelectionMenu(rEngine);
	ConditionallyRenderQuitConfirmation(rEngine);
	ConditionallyRenderOptions(rEngine);

	ImGui::EndChild();

	ImGui::End();
}

void GUI::ConditionallyRenderQuitConfirmation(Engine& rEngine)
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("CONFIRM QUIT", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure?");
		if (ImGui::Button("YES", ImVec2(120, 0)))
		{
			glfwSetWindowShouldClose(rEngine.m_pMainWindow->m_pGLFWwindow, true);	// it's all ogre now
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("CANCEL", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void GUI::ConditionallyRenderVisualizationSelectionMenu(Engine& rEngine)
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("VISUALIZATIONS", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("BOUNDING VOLUME HIERARCHY", ImVec2(0, 0)))
		{
			// CD relevant
			rEngine.m_pVisualization = new Visualization(*Engine::GetMainWindow());
			rEngine.m_pVisualization->Load();	// really bad implementation of "loading"

			// UI relevant
			//m_bCaptureMouse = false;
			ImGui::CloseCurrentPopup();
			m_bShowMainMenu = false;
		}

		if (ImGui::Button("BACK", ImVec2(0, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void GUI::ConditionallyRenderOptions(Engine& rEngine)
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("OPTIONS", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("BACK", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void GUI::HelpMarker(const char * sText)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(sText);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}