#include "GUI.h"

#include "Window.h"
#include "Engine.h"

#include <stdio.h>

GUI::GUI():
	m_bShowMainMenu(true),
	m_bShowSimulationControlPanel(false),
	m_bShowSimulationOptions(false),
	m_bCaptureMouse(true)
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
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_bShowSimulationControlPanel)
		RenderSimControlPanel(rEngine);

	if (m_bShowMainMenu)
		RenderMainMenu(rEngine);

	if (m_bShowSimulationOptions)
		RenderSimOptions(rEngine);

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
	ConditionallyRenderVisualizationSelectionMenu(rEngine);
	ConditionallyRenderQuitConfirmation(rEngine);
	ConditionallyRenderOptions(rEngine);

	ImGui::EndChild();

	ImGui::End();
}

void GUI::RenderSimControlPanel(Engine& rEngine)
{
	// configure window
	ImVec2 simControlWindowSize(500, 100);
	float fSimControlWindowPaddingBot = 0.0f;
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + main_viewport->Size.x * 0.5f - simControlWindowSize.x * 0.5f, main_viewport->WorkPos.y + main_viewport->Size.y - simControlWindowSize.y - fSimControlWindowPaddingBot), ImGuiCond_Always);
	ImGui::SetNextWindowSize(simControlWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if(m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;
	
	ImGui::Begin("Sim Controls", nullptr, window_flags);

	if (ImGui::Button("RESET"))
		rEngine.m_tVisualization.ResetSimulation();
	ImGui::SameLine();
	if (ImGui::Button("PLAY"))
		rEngine.m_tVisualization.PlaySimulation();
	ImGui::SameLine();
	if (ImGui::Button("PAUSE"))
		rEngine.m_tVisualization.PauseSimulation();
	ImGui::SameLine();
	if (ImGui::Button("STEP"))
		rEngine.m_tVisualization.MoveToNextSimulationStep();
	ImGui::SameLine();
	if (ImGui::Button("INVERT"))
		rEngine.m_tVisualization.InvertSimulationProgression();
	ImGui::SameLine();
	if (ImGui::Button("FASTER"))
		rEngine.m_tVisualization.IncreaseSimulationSpeed();
	ImGui::SameLine();
	if (ImGui::Button("SLOWER"))
		rEngine.m_tVisualization.DecreaseSimulationSpeed();
	
	if (ImGui::Button("OPTIONS"))
		m_bShowSimulationOptions = true;


	ImGui::End();
}

void GUI::RenderSimOptions(Engine& rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 simControlWindowSize(250, main_viewport->WorkSize.y);
	
	ImGui::SetNextWindowPos(ImVec2(0.0f, 00.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(simControlWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin("Sim Options", &m_bShowSimulationOptions, window_flags);

	// The combo box to choose a BVH construction strategy
	const char* pItems[] = { "TOP DOWN", "BOTTOM UP" };
	int iCurrentItemIndex = static_cast<int>(rEngine.m_tVisualization.GetCurrenBVHConstructionStrategy());
	const char* sComboLabel = pItems[iCurrentItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("BVH Construction Strategy", sComboLabel))
	{
		for (int iCurrentItem = 0; iCurrentItem < IM_ARRAYSIZE(pItems); iCurrentItem++)
		{
			const bool bIsSelected = (iCurrentItemIndex == iCurrentItem);
			if (ImGui::Selectable(pItems[iCurrentItem], bIsSelected))
			{
				assert(iCurrentItem <= static_cast<int>(Visualization::eBVHConstructionStrategy::SIZE));
				rEngine.m_tVisualization.SetNewBVHConstructionStrategy(static_cast<Visualization::eBVHConstructionStrategy>(iCurrentItem));
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (bIsSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGuiColorEditFlags iColorPickerFlags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoInputs;// | ImGuiColorEditFlags_NoLabel;

	// now follow options that are specific to certain construction strategies
	ImGui::Separator();

	// TOP DOWN OPTIONS
	if (iCurrentItemIndex == 0) 
	{
		ImGui::Text("TOP DOWN OPTIONS AND PARAMETERS");
		ImGui::ColorEdit3("Node Color##TOPDOWN", (float*)&rEngine.m_tVisualization.m_vec4TopDownNodeRenderColor, iColorPickerFlags);
	}

	// BOTTOM UP OPTIONS
	if (iCurrentItemIndex == 1)
	{
		ImGui::Text("BOTTOM UP OPTIONS AND PARAMETERS");
		ImGui::ColorEdit3("Node Color##BOTTOMUP", (float*)&rEngine.m_tVisualization.m_vec4BottomUpNodeRenderColor, iColorPickerFlags);
	}
	
	ImGui::Separator();
	// now follow options that are generally available in the visualization

	ImGui::Text("General");
	ImGui::ColorEdit3("Draw Color##AABB", (float*)&rEngine.m_tVisualization.m_vec4AABBDefaultColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Draw AABBs of Objects", &rEngine.m_tVisualization.m_bRenderObjectAABBs);
	ImGui::ColorEdit3("Draw Color##BoundingSphere", (float*)&rEngine.m_tVisualization.m_vec4BoundingSphereDefaultColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Draw Bounding Spheres of Objects", &rEngine.m_tVisualization.m_bRenderObjectBoundingSpheres);
	
	ImGui::Separator();

	ImGui::Text("X Axis");
	ImGui::ColorEdit3("Draw Color##X", (float*)&rEngine.m_tVisualization.m_vec4GridColorX, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Draw Grid##checkBoxXGrid", &rEngine.m_tVisualization.m_bRenderGridXPlane);
	ImGui::Text("Y Axis");
	ImGui::ColorEdit3("Draw Color##Y", (float*)&rEngine.m_tVisualization.m_vec4GridColorY, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Draw Grid##checkBoxYGrid", &rEngine.m_tVisualization.m_bRenderGridYPlane); 
	ImGui::Text("Z Axis");
	ImGui::ColorEdit3("Draw Color##Z", (float*)&rEngine.m_tVisualization.m_vec4GridColorZ, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Draw Grid##checkBoxZGrid", &rEngine.m_tVisualization.m_bRenderGridZPlane); 
	
	ImGui::Separator();
	ImGui::Text("Crooshair");
	ImGui::ColorEdit3("Draw Color##XHAIR", (float*)&rEngine.m_tVisualization.m_vec4CrossHairColor, iColorPickerFlags);
	ImGui::SliderFloat("Crosshair Size", &rEngine.m_tVisualization.m_fCrossHairScaling, 0.75f, 2.0f, "%.2f", ImGuiSliderFlags_NoInput);

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
			glfwSetWindowShouldClose(rEngine.m_tWindow.m_pGLFWwindow, true);
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
		if (ImGui::Button("BOUNDING VOLUME HIERARCHY", ImVec2(120, 0)))
		{
			// CD relevant
			rEngine.m_tVisualization.Load();	// really bad implementation
			CollisionDetection::ConstructBoundingVolumesForScene(rEngine.m_tVisualization);
			CollisionDetection::UpdateBoundingVolumesForScene(rEngine.m_tVisualization);
			rEngine.m_tTopDownBVH = CollisionDetection::ConstructTopDownBVHForScene(rEngine.m_tVisualization);
			rEngine.m_tBottomUpBVH = CollisionDetection::ConstructBottomUPBVHForScene(rEngine.m_tVisualization);

			// UI relevant
			rEngine.m_tWindow.SetMouseCaptured(true);
			ShowMenu(false);
			m_bShowSimulationControlPanel = true;
			m_bCaptureMouse = false;
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("BACK", ImVec2(120, 0)))
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