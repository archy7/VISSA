#include "GUI.h"

#include "Window.h"

#include <stdio.h>

GUI::GUI():
	m_bShowMainMenu(true),
	m_bShowSimulationControlPanel(false)
{

}

void GUI::InitForWindow(Window& rWindow)
{
	assert(rWindow.m_bIsInitialized);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(rWindow.m_pGLFWwindow, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void GUI::ToggleMenuState()
{
	m_bShowMainMenu = !m_bShowMainMenu;
}

bool GUI::IsMenuActive() const
{
	return m_bShowMainMenu;
}

void GUI::Render(Window& rWindow)
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_bShowSimulationControlPanel)
		RenderSimControlPanel(rWindow);

	if (m_bShowMainMenu)
		RenderMainMenu(rWindow);



	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::RenderMainMenu(Window & rWindow)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImGui::SetNextWindowPos(main_viewport->WorkPos);
	ImGui::SetNextWindowSize(main_viewport->WorkSize);


	ImGuiWindowFlags backgroundFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs;
	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	ImGui::Begin("Main Menu Background", NULL, backgroundFlags);

	ImGui::Text("TRANSPARENT MENU BACK GROUND");

	ImGuiWindowFlags menuFlags = 0;
	ImVec2 mainMenuWindowSize(ImGui::GetWindowContentRegionWidth() * 0.2f, 400);
	ImVec2 mainMenuWindowPosition(main_viewport->WorkSize.x * 0.5f - mainMenuWindowSize.x * 0.5f, main_viewport->WorkSize.y * 0.5f - mainMenuWindowSize.y * 0.5f);
	ImGui::SetNextWindowPos(mainMenuWindowPosition);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::BeginChild("Main Menu", mainMenuWindowSize, true, menuFlags);

	ImGui::Text("MENU");

	// these buttons will open another window when pressed, see below
	if (ImGui::Button("VISUALIZATIONS"))
	{
		ImGui::OpenPopup("VISUALIZATIONS");
	}
	if (ImGui::Button("OPTIONS"))
	{
		ImGui::OpenPopup("OPTIONS");
	}
	if (ImGui::Button("QUIT"))
	{
		ImGui::OpenPopup("CONFIRM QUIT");
	}

	// Conditions are internally handled by the GUI library and set to true by the above ::OpenPopup() commands
	ConditionallyRenderQuitConfirmation(rWindow);
	ConditionallyRenderOptions(rWindow);
	ConditionallyRenderVisualizationSelectionMenu(rWindow);

	ImGui::EndChild();

	ImGui::End();
}

void GUI::RenderSimControlPanel(Window & rWindow)
{
	// configure window
	ImVec2 simControlWindowSize(400, 100);
	float fSimControlWindowPaddingBot = 50.0f;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x * 0.5f - simControlWindowSize.x * 0.5f, main_viewport->WorkPos.y + main_viewport->Size.y - simControlWindowSize.y - fSimControlWindowPaddingBot), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(simControlWindowSize, ImGuiCond_FirstUseEver);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	//if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	//if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	//if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	//if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
	//if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
	//if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
	//if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
	//if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
	//if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	//if (no_close)           p_open = NULL; // Don't pass our bool* to Begin
	ImGui::Begin("Sim Controls", nullptr, window_flags);

	if (ImGui::Button("RESET"))
		fprintf(stdout, "RESET\n");
	ImGui::SameLine();
	if (ImGui::Button("PLAY"))
		fprintf(stdout, "PLAY\n");
	ImGui::SameLine();
	if (ImGui::Button("PAUSE"))
		fprintf(stdout, "PAUSE\n");
	ImGui::SameLine();
	if (ImGui::Button("STEP"))
		fprintf(stdout, "STEP\n");
	ImGui::SameLine();
	if (ImGui::Button("REWIND"))
		fprintf(stdout, "REWIND\n");
	ImGui::SameLine();
	if (ImGui::Button("FAST FORWARD"))
		fprintf(stdout, "FAST FORWARD\n");
	ImGui::SameLine();

	ImGui::End();
}

void GUI::ConditionallyRenderQuitConfirmation(Window & rWindow)
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("CONFIRM QUIT", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure?");
		if (ImGui::Button("YES", ImVec2(120, 0)))
		{
			glfwSetWindowShouldClose(rWindow.m_pGLFWwindow, true);

		}
		ImGui::SameLine();
		if (ImGui::Button("CANCEL", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void GUI::ConditionallyRenderVisualizationSelectionMenu(Window & rWindow)
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("VISUALIZATIONS", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::Button("BACK", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void GUI::ConditionallyRenderOptions(Window & rWindow)
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