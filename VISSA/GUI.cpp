#include "GUI.h"

#include "Window.h"
#include "Engine.h"

#include <stdio.h>

namespace {
	template <typename T>
	struct GenericBackup {
		GenericBackup() :
			m_tBackedupData(),
			m_bValid(false)
		{};
		GenericBackup(T tOriginalData) :
			m_tBackedupData(tOriginalData),
			m_bValid(true)
		{

		}

		T m_tBackedupData;
		bool m_bValid;
	};

	GenericBackup<SceneObject::Transform> tObjectPropertiesBackup;

	// TODO: reconsider this. might be a useful idea in the future. right now, its overkill
	/*template <typename T>
	struct GenericViewModel {
		T m_tModelData;
	};

	GenericViewModel<SceneObject> m_tSceneObject;*/

	struct SceneObjectViewModel {
		SceneObject m_tSceneObject;
		int iCurrentlySelectedDropDownIndex = 0;
	} tObjectCreationViewModel;
	

	void ResetObjectCreationViewModel(){
		tObjectCreationViewModel.m_tSceneObject = SceneObject();
		tObjectCreationViewModel.iCurrentlySelectedDropDownIndex = 0;
	}

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	void HelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}
}

GUI::GUI():
	m_bShowMainMenu(true),
	m_bShowSimulationControlPanel(false),
	m_bShowSimulationOptions(false),
	m_bCaptureMouse(true),
	m_bShowObjectPropertiesWindow(false),
	m_bShowObjectCreationWindow(false),
	m_bShowHelpWindow(false),
	m_bObjectPropertiesPendingChanges(false)
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

void GUI::ShowObjectPropertiesWindow(bool bShowIt)
{
	m_bShowObjectPropertiesWindow = bShowIt;

	// tell the backup that it needs an update
	tObjectPropertiesBackup.m_bValid = false;
}

void GUI::ToggleHelpWindow()
{
	m_bShowHelpWindow = !m_bShowHelpWindow;
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

void GUI::SetObjectPropertiesWindowPosition(float fXPosition, float fYPosition)
{
	m_vec2ObjectPropertiesWindowPosition = ImVec2(fXPosition, fYPosition);
}

void GUI::SetFocusedObject(SceneObject * pFocusedObject)
{
	// nullptr for paramenter is valid -> no object in focus
	m_pCurrentlyFocusedObject = pFocusedObject;
}

SceneObject * GUI::GetFocusedObject()
{
	return m_pCurrentlyFocusedObject;
}

void GUI::CancelObjectPropertiesChanges()
{
	assert(m_pCurrentlyFocusedObject);
	if(m_bObjectPropertiesPendingChanges)
		m_pCurrentlyFocusedObject->m_tTransform = tObjectPropertiesBackup.m_tBackedupData;

	m_bShowObjectPropertiesWindow = false; // closes this window
	tObjectPropertiesBackup.m_bValid = false; // future backup data will need to be fetched again
	m_bObjectPropertiesPendingChanges = false; // reset this flag
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

	if (m_bShowObjectPropertiesWindow)
		RenderObjectPropertiesWindow(rEngine);

	if (m_bShowObjectCreationWindow)
		RenderObjectCreationWindow(rEngine);

	if (m_bShowHelpWindow)
		RenderHelpWindow(rEngine);

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

void GUI::RenderSimControlPanel(Engine& rEngine)
{
	// configure window
	ImVec2 simControlWindowSize(500, 120);
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
	
	std::string sControlPanelName("CONTROLS for: ");

	switch (rEngine.m_tVisualization.GetCurrenBVHConstructionStrategy())
	{
	case Visualization::eBVHConstructionStrategy::TOPDOWN:
		sControlPanelName.append("TOP DOWN ");
		break;
	case Visualization::eBVHConstructionStrategy::BOTTOMUP:
		sControlPanelName.append("BOTTOM UP ");
		break;
	default:
		assert(!"disaster");
		break;
	}

	switch (rEngine.m_tVisualization.GetCurrentBVHBoundingVolume())
	{
	case Visualization::eBVHBoundingVolume::AABB:
		sControlPanelName.append("AABBs");
		break;
	case Visualization::eBVHBoundingVolume::BOUNDING_SPHERE:
		sControlPanelName.append("Bounding Spheres");
		break;
	default:
		assert(!"disaster");
		break;
	}

	ImGui::Begin(sControlPanelName.c_str(), nullptr, window_flags);

	if (ImGui::Button("RESET"))
		rEngine.m_tVisualization.ResetSimulation();
	ImGui::SameLine();

	if (rEngine.m_tVisualization.m_ePresentationMode == Visualization::ePresentationMode::DISCRETE)
	{
		if (ImGui::Button("PLAY"))
			rEngine.m_tVisualization.PlaySimulation();
	}
	else
	{
		if (ImGui::Button("PAUSE"))
			rEngine.m_tVisualization.PauseSimulation();
	}
	ImGui::SameLine();
	if (ImGui::Button("STEP"))
		rEngine.m_tVisualization.MoveToNextSimulationStep();
	ImGui::SameLine();
	std::string sInverButtonLabel = (rEngine.m_tVisualization.m_iSimulationDirectionSign == 1) ? "REVERSE PLAYBACK" : "FORWARD PLAYBACK";
	if (ImGui::Button(sInverButtonLabel.c_str()))
		rEngine.m_tVisualization.InvertSimulationProgression();


	if (ImGui::Button("SLOWER"))
		rEngine.m_tVisualization.DecreaseSimulationSpeed();
	ImGui::SameLine();
	ImGui::Text("Speed: %.2f", rEngine.m_tVisualization.GetCurrentSimulationSpeed());
	ImGui::SameLine();
	if (ImGui::Button("FASTER"))
		rEngine.m_tVisualization.IncreaseSimulationSpeed();

	ImGui::Separator();
	if (ImGui::Button("SIMULATION OPTIONS"))
		m_bShowSimulationOptions = !m_bShowSimulationOptions;
	ImGui::SameLine();
	if (ImGui::Button("HELP"))
		ToggleHelpWindow();

	ImGui::End();
}

void GUI::RenderSimOptions(Engine& rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 simOptionswindowSize(250, main_viewport->WorkSize.y);
	
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(simOptionswindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin("Options", &m_bShowSimulationOptions, window_flags);

	ImGui::Text("Scene");
	if (ImGui::Button("Load Scene"))
	{
		ImGui::OpenPopup("Select Scene");
	}
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Select Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::Button("Default Scene", ImVec2(120, 0)))
			{
				rEngine.m_tVisualization.LoadDefaultScene();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine(); HelpMarker("The scene which is loaded when you first start the Bounding Volume Hierarchy Visualization");

			if (ImGui::Button("CANCEL", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	if (ImGui::Button("Add Object"))
	{
		m_bShowObjectCreationWindow = true;
	}

	if (ImGui::Button("Delete All Objects"))
	{
		ImGui::OpenPopup("CONFIRM DELETION OF ALL OBJECTS");
	}
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("CONFIRM DELETION OF ALL OBJECTS", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure?"); ImGui::SameLine(); HelpMarker("Deleting all objects will reconstruct the Bounding Volume Hierarchy and resest the simulation.");
			if (ImGui::Button("YES", ImVec2(120, 0)))
			{
				rEngine.m_tVisualization.ClearCurrentScene();
				m_bShowObjectPropertiesWindow = false;
				ImGui::CloseCurrentPopup();
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

	ImGui::Separator();
	ImGui::Text("BVH");

	ImGui::Text("BVH Bounding Volume"); ImGui::SameLine(); HelpMarker("The Hierarchy's nodes' Bounding Volume");
	// The combo box to choose the hierarchy's node bounding volume
	const char* pBVHBoundingVolumeItems[] = { "AABB", "Bounding Sphere" };
	int iCurrentBVHBoundingVolumeItemIndex = static_cast<int>(rEngine.m_tVisualization.GetCurrentBVHBoundingVolume());
	const char* sBVHBoundingVolumeComboLabel = pBVHBoundingVolumeItems[iCurrentBVHBoundingVolumeItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("##BVH Bounding Volume", sBVHBoundingVolumeComboLabel))
	{
		for (int iCurrentItem = 0; iCurrentItem < IM_ARRAYSIZE(pBVHBoundingVolumeItems); iCurrentItem++)
		{
			const bool bIsSelected = (iCurrentBVHBoundingVolumeItemIndex == iCurrentItem);
			if (ImGui::Selectable(pBVHBoundingVolumeItems[iCurrentItem], bIsSelected))
			{
				assert(iCurrentItem <= static_cast<int>(Visualization::eBVHBoundingVolume::NUM_BVHBOUNDINGVOLUMES));
				rEngine.m_tVisualization.SetNewBVHBoundingVolume(static_cast<Visualization::eBVHBoundingVolume>(iCurrentItem));
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (bIsSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Construction Strategy");
	// The combo box to choose a BVH construction strategy
	const char* pBVHConstructionStrategyItems[] = { "TOP DOWN", "BOTTOM UP" };
	int iCurrentConstructionStrategyItemIndex = static_cast<int>(rEngine.m_tVisualization.GetCurrenBVHConstructionStrategy());
	const char* sConstructionStrategyComboLabel = pBVHConstructionStrategyItems[iCurrentConstructionStrategyItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("##BVH Construction Strategy", sConstructionStrategyComboLabel))
	{
		for (int iCurrentItem = 0; iCurrentItem < IM_ARRAYSIZE(pBVHConstructionStrategyItems); iCurrentItem++)
		{
			const bool bIsSelected = (iCurrentConstructionStrategyItemIndex == iCurrentItem);
			if (ImGui::Selectable(pBVHConstructionStrategyItems[iCurrentItem], bIsSelected))
			{
				assert(iCurrentItem <= static_cast<int>(Visualization::eBVHConstructionStrategy::NUM_BVHCONSTRUCTIONSTRATEGIES));
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

	// TOP DOWN OPTIONS
	if (iCurrentConstructionStrategyItemIndex == 0) 
	{
		ImGui::Text("TOP DOWN OPTIONS AND PARAMETERS");
		ImGui::ColorEdit3("Node Color##TOPDOWN", (float*)&rEngine.m_tVisualization.m_vec4TopDownNodeRenderColor, iColorPickerFlags); ImGui::SameLine();
		ImGui::Checkbox("Gradient##TOPDOWN", &rEngine.m_tVisualization.m_bNodeDepthColorGrading);
		if(rEngine.m_tVisualization.m_bNodeDepthColorGrading)
			ImGui::ColorEdit3("Node Gradient Color##TOPDOWN", (float*)&rEngine.m_tVisualization.m_vec4TopDownNodeRenderColor_Gradient, iColorPickerFlags);
	}

	// BOTTOM UP OPTIONS
	if (iCurrentConstructionStrategyItemIndex == 1)
	{
		ImGui::Text("BOTTOM UP OPTIONS AND PARAMETERS");
		ImGui::ColorEdit3("Node Color##BOTTOMUP", (float*)&rEngine.m_tVisualization.m_vec4BottomUpNodeRenderColor, iColorPickerFlags); ImGui::SameLine();
		ImGui::Checkbox("Gradient##BOTTOMUP", &rEngine.m_tVisualization.m_bNodeDepthColorGrading);
		if (rEngine.m_tVisualization.m_bNodeDepthColorGrading)
			ImGui::ColorEdit3("Node Gradient Color##BOTTOMUP", (float*)&rEngine.m_tVisualization.m_vec4BottomUpNodeRenderColor_Gradient, iColorPickerFlags);
	}

	

	//if (ImGui::Button("Rebuild BVHs"))
	//{
	//	CollisionDetection::UpdateBoundingVolumesForScene(rEngine.m_tVisualization);
	//	rEngine.m_tTopDownBVH_AABB = CollisionDetection::ConstructTopDownAABBBVHForScene(rEngine.m_tVisualization);
	//	rEngine.m_tBottomUpBVH_AABB = CollisionDetection::ConstructBottomUpAABBBVHForScene(rEngine.m_tVisualization);
	//	rEngine.m_tVisualization.m_bBVHTreesValid = true;
	//}
	
	ImGui::Separator();
	
	ImGui::Text("Object AABBs");
	ImGui::ColorEdit3("Color##AABB", (float*)&rEngine.m_tVisualization.m_vec4AABBDefaultColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##Draw AABBs of Objects", &rEngine.m_tVisualization.m_bRenderObjectAABBs);
	ImGui::Text("Object Bounding Spheres");
	ImGui::ColorEdit3("Color##BoundingSphere", (float*)&rEngine.m_tVisualization.m_vec4BoundingSphereDefaultColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##Draw Bounding Spheres of Objects", &rEngine.m_tVisualization.m_bRenderObjectBoundingSpheres);
	
	ImGui::Separator();

	// X axis grid
	ImGui::Text("X Axis Grid"); ImGui::SameLine(); HelpMarker("A rasterized grid facing the X axis, spanning the YZ plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##X", (float*)&rEngine.m_tVisualization.m_vec4GridColorX, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxXGrid", &rEngine.m_tVisualization.m_bRenderGridXPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Xdown", ImGuiDir_Down)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.x -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Xup", ImGuiDir_Up)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.x += 10.0f; }
	ImGui::PopButtonRepeat();
	// Y axis grid
	ImGui::Text("Y Axis Grid"); ImGui::SameLine(); HelpMarker("A rasterized grid facing the Y axis, spanning the XZ plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##Y", (float*)&rEngine.m_tVisualization.m_vec4GridColorY, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxYGrid", &rEngine.m_tVisualization.m_bRenderGridYPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Ydown", ImGuiDir_Down)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.y -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Yup", ImGuiDir_Up)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.y += 10.0f; }
	ImGui::PopButtonRepeat();
	// Z axis grid
	ImGui::Text("Z Axis Grid"); ImGui::SameLine(); HelpMarker("A rasterized grid facing the Z axis, spanning the XY plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##Z", (float*)&rEngine.m_tVisualization.m_vec4GridColorZ, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxZGrid", &rEngine.m_tVisualization.m_bRenderGridZPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Zdown", ImGuiDir_Down)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.z -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Zup", ImGuiDir_Up)) { rEngine.m_tVisualization.m_vec3GridPositionsOnAxes.z += 10.0f; }
	ImGui::PopButtonRepeat();
	
	ImGui::Separator();
	ImGui::Text("Crosshair");
	ImGui::ColorEdit3("Color##XHAIR", (float*)&rEngine.m_tVisualization.m_vec4CrossHairColor, iColorPickerFlags);
	ImGui::SliderFloat("Size", &rEngine.m_tVisualization.m_fCrossHairScaling, 0.75f, 2.0f, "%.2f", ImGuiSliderFlags_NoInput);

	ImGui::End();
}

void GUI::RenderObjectPropertiesWindow(Engine & rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(350, 250);

	ImGui::SetNextWindowPos(m_vec2ObjectPropertiesWindowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin("Object Properties", nullptr, window_flags);

	assert(m_pCurrentlyFocusedObject);
	
	switch (m_pCurrentlyFocusedObject->m_eType)
	{
	case SceneObject::eType::CUBE: {
		ImGui::Text("Type: CUBE");
		break;
	}
	case SceneObject::eType::SPHERE: {
		ImGui::Text("Type: SPHERE");
		break;
	}
	default:
		assert(!"disaster struck");
	}

	if (tObjectPropertiesBackup.m_bValid == false) // if the backup still holds data of an old object, update it
	{
		tObjectPropertiesBackup.m_tBackedupData = m_pCurrentlyFocusedObject->m_tTransform;
		tObjectPropertiesBackup.m_bValid = true;
	}	

	ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal;// | ImGuiInputTextFlags_EnterReturnsTrue;
	if (ImGui::InputFloat3("Position", &(m_pCurrentlyFocusedObject->m_tTransform.m_vec3Position.x), "%.2f", flags))
		m_bObjectPropertiesPendingChanges = true;
	ImGui::SameLine(); HelpMarker("[X][Y][Z]");
	if(ImGui::InputFloat3("Scale", &(m_pCurrentlyFocusedObject->m_tTransform.m_vec3Scale.x), "%.2f", flags))
		m_bObjectPropertiesPendingChanges = true;
	ImGui::SameLine(); HelpMarker("[X][Y][Z]");
	if(ImGui::InputFloat4("Rotation", &(m_pCurrentlyFocusedObject->m_tTransform.m_tRotation.m_vec3Axis.x), "%.2f", flags))
		m_bObjectPropertiesPendingChanges = true;
	ImGui::SameLine(); HelpMarker("[X][Y][Z][angle]");
	if (ImGui::Button("DELETE OBJECT"))
	{
		ImGui::OpenPopup("CONFIRM OBJECT DELETION");
	}
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("CONFIRM OBJECT DELETION", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure?"); ImGui::SameLine(); HelpMarker("Deleting this object will reconstruct the Bounding Volume Hierarchy and resest the simulation. It cannot be undone.");
			if (ImGui::Button("YES", ImVec2(120, 0)))
			{
				m_bShowObjectPropertiesWindow = false; // closes the window

				rEngine.m_tVisualization.DeleteGivenObject(m_pCurrentlyFocusedObject);
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

	
	if (m_bObjectPropertiesPendingChanges)
		ImGui::TextWrapped("Applying changes to object properties [OK] will invalidate the BVH. It will automatically be reconstructed and the visualization reset. Clicking [CANCEL] will discard all changes.");

	if (ImGui::Button("OK"))
	{
		m_bShowObjectPropertiesWindow = false; // closes this window
		tObjectPropertiesBackup.m_bValid = false; // backup data will need to be fetched again

		if (m_bObjectPropertiesPendingChanges) // only update the trees if changes were commited
		{
			rEngine.m_tVisualization.UpdateAfterObjectPropertiesChange();
		}

		m_bObjectPropertiesPendingChanges = false; // reset this flag
		
	}
	ImGui::SameLine();
	if (ImGui::Button("CANCEL"))
	{
		
		// rolling back changes
		CancelObjectPropertiesChanges();
		

		
	}

	ImGui::End();
}

void GUI::RenderObjectCreationWindow(Engine & rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(350, 250);

	ImGui::SetNextWindowPos(main_viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin("Object Creation", nullptr, window_flags);

	SceneObject* pObjectCreationData = &tObjectCreationViewModel.m_tSceneObject;

	// The combo box to choose a BVH construction strategy
	const char* pItems[] = { "CUBE", "SPHERE" };
	int iCurrentlySelectedItemIndex = static_cast<int>(tObjectCreationViewModel.iCurrentlySelectedDropDownIndex);
	const char* sComboLabel = pItems[iCurrentlySelectedItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("##BVH Construction Strategy", sComboLabel))
	{
		for (int iCurrentItemIndex = 0; iCurrentItemIndex < IM_ARRAYSIZE(pItems); iCurrentItemIndex++)
		{
			const bool bIsSelected = (iCurrentlySelectedItemIndex == iCurrentItemIndex);
			if (ImGui::Selectable(pItems[iCurrentItemIndex], bIsSelected))
			{
				// TODO: this is really ugly
				if (iCurrentItemIndex == 0)
				{
					tObjectCreationViewModel.iCurrentlySelectedDropDownIndex = iCurrentItemIndex;
					pObjectCreationData->m_eType = SceneObject::eType::CUBE;
				}	
				else if (iCurrentItemIndex == 1)
				{
					tObjectCreationViewModel.iCurrentlySelectedDropDownIndex = iCurrentItemIndex;
					pObjectCreationData->m_eType = SceneObject::eType::SPHERE;
				}
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (bIsSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}


	ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsDecimal;
	ImGui::InputFloat3("Position", &(pObjectCreationData->m_tTransform.m_vec3Position.x), "%.2f", flags);
	ImGui::SameLine(); HelpMarker("[X][Y][Z]");
	ImGui::InputFloat3("Scale", &(pObjectCreationData->m_tTransform.m_vec3Scale.x), "%.2f", flags);
	ImGui::SameLine(); HelpMarker("[X][Y][Z]");
	ImGui::InputFloat4("Rotation", &(pObjectCreationData->m_tTransform.m_tRotation.m_vec3Axis.x), "%.2f", flags);
	ImGui::SameLine(); HelpMarker("[X][Y][Z][angle]");
	
	ImGui::TextWrapped("Creating a new object [OK] will invalidate the BVH. It will automatically be reconstructed and the visualization reset.");

	if (ImGui::Button("OK"))
	{
		rEngine.m_tVisualization.AddNewSceneObject(tObjectCreationViewModel.m_tSceneObject);

		m_bShowObjectCreationWindow = false; // closes this window
		ResetObjectCreationViewModel();
	}
	ImGui::SameLine();
	if (ImGui::Button("CANCEL"))
	{
		m_bShowObjectCreationWindow = false; // closes this window
		ResetObjectCreationViewModel();
	}

	ImGui::End();
}

void GUI::RenderHelpWindow(Engine & rEngine)
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(450, 450);

	ImGui::SetNextWindowPos(main_viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bCaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin("Help", &m_bShowHelpWindow, window_flags);
	//ImGui::SetWindowFocus();

	ImGui::Text("Visualization for Bounding Volume Hierarchies");
	ImGui::Separator();
	ImGui::Text("Controls");
	ImGui::Text("[ESC] : Opens Main Menu");
	ImGui::Text("[W][A][S][D] : Move Camera [FORWARD][LEFT][BACK][RIGHT]");
	ImGui::Text("[Q],[E] : Move Camera [UP][DOWN]");
	ImGui::Text("[M] : Toggle between mouse cursor and camera control mode");
	ImGui::Text("[MOVE MOUSE] : Look around (in camera control mode)");	
	ImGui::Text("[LEFT MOUSE] : Select object and show its properties");
	ImGui::Text("[H] : Show this Help Window");
	ImGui::Separator();
	ImGui::Text("Context");
	ImGui::TextWrapped("If you need a quick introduction or refresher to Bounding Volume Hierarchies on what they are and what they are good for, you may at any time refer to VISSA's user manual and knowledge base. This visualization's focus is to show a few ways how BVH's can be constructed for a given scene.");
	ImGui::TextWrapped("Feel free to explore: Edit the scene, delete objects, create new objects... and see how the resulting BVH changes.");
	ImGui::TextWrapped("At the bottom of the screen, you can find the control panel for the visualization. From there, you can also open further visualization options and this window.");
	ImGui::Separator();
	ImGui::TextWrapped("Have Fun and Good Learning!");

	if (ImGui::Button("OK"))
	{
		m_bShowHelpWindow = false;
	}

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
			rEngine.m_tVisualization.Load();	// really bad implementation of "loading"

			// UI relevant
			//rEngine.m_tMainWindow.SetHardCaptureMouse(true);
			ShowMenu(false);
			ToggleHelpWindow();
			m_bShowSimulationControlPanel = true;
			//m_bCaptureMouse = false;
			ImGui::CloseCurrentPopup();
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