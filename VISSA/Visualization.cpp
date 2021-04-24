#include "Visualization.h"

#include <assert.h>
#include <algorithm>
#include <limits>
#include <cmath>

#include "glm/glm.hpp"

#include "Engine.h"
#include "GeometricPrimitiveData.h"
#include "Renderer.h"


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


	void ResetObjectCreationViewModel() {
		tObjectCreationViewModel.m_tSceneObject = SceneObject();
		tObjectCreationViewModel.iCurrentlySelectedDropDownIndex = 0;
	}
}


Visualization::Visualization(Window& rMainWindow) :
	m_rMainWindow(rMainWindow),
	m_tCamera(glm::vec3(0.0f, 0.0f, 0.0f)),
	m_mat4Camera(glm::mat4(1.0f)),
	m_mat4PerspectiveProjection3DWindow(glm::mat4(1.0f)),
	m_mat4OrthographicProjection3DWindow(glm::mat4(1.0f)),
	m_mat4OrthographicProjection2DWindow(glm::mat4(1.0f)),
	m_p2DGraphWindow(nullptr),
	m_iMaximumRenderedTreeDepth(),
	m_iNumberStepsRendered(0),
	m_ePresentationMode(DISCRETE),
	m_eConstructionStrategy(TOPDOWN),
	m_eBVHBoundingVolume(AABB),
	m_pCurrentlyActiveConstructionStrategy(nullptr),
	m_pCurrentlyFocusedObject(nullptr),
	m_fCrossHairScaling(1.0f),
	m_fRenderDistance(10000.0f),
	m_uiCurrentPlayBackSpeedIndex(0u),
	m_iSimulationDirectionSign(0),
	m_fAccumulatedTimeSinceLastUpdateStep(0.0f),
	m_f2DGraphNodeSize(0.0f),
	m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel(0.0f),
	m_bRenderObjectAABBs(false),
	m_bRenderObjectBoundingSpheres(false),
	m_bRenderGridXPlane(false),
	m_bRenderGridYPlane(false),
	m_bRenderGridZPlane(false),
	m_bNodeDepthColorGrading(false),
	m_bGUICaptureMouse(true),
	m_bShowSimulationOptions(false),
	m_bShowObjectCreationWindow(false),
	m_bShowObjectPropertiesWindow(false),
	m_bShowHelpWindow(true),
	m_bObjectPropertiesPendingChanges(false),
	m_fDeltaTime(0.0f)
{
	assert(&m_rMainWindow == Engine::GetMainWindow()); // just to protect from mistakes (duh). the ctor expects the main window to make the dependency clear
	glfwSetWindowTitle(m_rMainWindow.m_pGLFWwindow, "Bounding Volume Hierarchy Visualization");

	InitPlaybackSpeeds();
	InitRenderColors();
	ResetSimulation();
}

Visualization::~Visualization()
{
	delete m_p2DGraphWindow;

	m_tTopDownAABBs.m_tBVH.DeleteTree();
	m_tBottomUpAABBs.m_tBVH.DeleteTree();
	m_tTopDownBoundingSpheres.m_tBVH.DeleteTree();
	m_tBottomUpBoundingSpheres.m_tBVH.DeleteTree();
}

void Visualization::Load()
{
	//m_p3DSceneWindow = Engine::MakeWindowAndSetAsContext(1280, 720, "Bounding Volume Hierarchy Visualization");
	m_p2DGraphWindow = Engine::MakeWindowAndSetAsContext(640, 480, "BVH Tree");


	// create all gl relevant resources for the context of the second window.
	// only container objects (https://www.khronos.org/opengl/wiki/OpenGL_Object#Container_objects) cannot be shared between contexts, and MUST be created for each context
	// However, for now the decision was made that no objects are going to be shared -> create all of them

	SetInitialRenderStates();
	glAssert();
	LoadShaders();
	glAssert();
	LoadTextures();
	glAssert();
	InitUniformBuffers();
	glAssert();
	LoadPrimitivesToGPU();
	glAssert();

	LoadDefaultScene();
	m_tCamera.SetToPosition(glm::vec3(0.0f, 0.0f, 1500.0f));
}

void Visualization::LoadDefaultScene()
{
	using namespace Primitives;

	ClearCurrentScene();

	std::vector<glm::vec3> vecNewObjectsPositions;
	std::vector<glm::vec3> vecNewObjectsScales;
	std::vector<SceneObject::Transform::Rotation> vecNewObjectsRotations;
	
	vecNewObjectsPositions.reserve(100);
	vecNewObjectsScales.reserve(100);
	vecNewObjectsRotations.reserve(100);

	// make new positions
	vecNewObjectsPositions.push_back(glm::vec3(150.0f, 100.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-200.0f, 100.0f, 0.0f));
	vecNewObjectsPositions.push_back(glm::vec3(200.0f, 250.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(275.0f, 250.0f, 100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(50.0f, 300.0f, 400.0f));
	vecNewObjectsPositions.push_back(glm::vec3(0.0f, 500.0f, -100.0f));
	vecNewObjectsPositions.push_back(glm::vec3(300.0f, -300.0f, 300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-300.0f, 250.0f, 500.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-100.0f, 500.0f, -300.0f));
	vecNewObjectsPositions.push_back(glm::vec3(-200.0f, 125.0f, -500.0f));

	// make new scales
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
	vecNewObjectsScales.push_back(glm::vec3(0.8f, 0.8f, 0.8f));
	vecNewObjectsScales.push_back(glm::vec3(1.5f, 1.5f, 1.5f));
	vecNewObjectsScales.push_back(glm::vec3(1.7f, 1.7f, 1.7f));
	vecNewObjectsScales.push_back(glm::vec3(0.9f, 0.9f, 0.9f));
	vecNewObjectsScales.push_back(glm::vec3(3.0f, 1.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(2.0f, 2.0f, 2.0f));
	vecNewObjectsScales.push_back(glm::vec3(0.4f, 0.4f, 0.4f));

	// make new rotations
	SceneObject::Transform::Rotation tNewRotation;
	tNewRotation.m_fAngle = 0.0f;
	tNewRotation.m_vec3Axis = glm::vec3(0.0f, 1.0f, 0.0f);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);
	vecNewObjectsRotations.push_back(tNewRotation);

	assert(vecNewObjectsPositions.size() == vecNewObjectsScales.size());
	assert(vecNewObjectsScales.size() == vecNewObjectsRotations.size());

	for (int uiCurrentNewObject = 0; uiCurrentNewObject < vecNewObjectsPositions.size(); uiCurrentNewObject++)
	//for (int uiCurrentNewObject = 0; uiCurrentNewObject < 4; uiCurrentNewObject++)
	{
		SceneObject tNewObject;
		tNewObject.m_tTransform.m_vec3Position = vecNewObjectsPositions[uiCurrentNewObject];
		tNewObject.m_tTransform.m_tRotation = vecNewObjectsRotations[uiCurrentNewObject];
		tNewObject.m_tTransform.m_vec3Scale = vecNewObjectsScales[uiCurrentNewObject];
		if (uiCurrentNewObject % 2 == 0)
		//if (uiCurrentNewObject == 0)
		{
			tNewObject.m_eType = SceneObject::eType::CUBE;
		}
		else
		{
			tNewObject.m_eType = SceneObject::eType::SPHERE;
		}

		m_vecObjects.push_back(tNewObject);
	}

	CollisionDetection::ConstructBoundingVolumesForScene(*this);
	CollisionDetection::UpdateBoundingVolumesForScene(*this);
	ReconstructAllTrees();
}

void Visualization::Render()
{
	assert(m_p2DGraphWindow);

	m_rMainWindow.SetAsCurrentRenderContext();

	UpdateFrameConstants();
	glAssert();
	UpdateProjectionMatrices();
	glAssert();
	Render3DVisualization();
	glAssert();

	m_p2DGraphWindow->SetAsCurrentRenderContext();

	Render2DGraph();
	glAssert();
	
}

void Visualization::ReconstructAllTrees()
{
	m_tTopDownAABBs.DeleteAllData();
	m_tTopDownAABBs = ConstructTopDownAABBBVHandRenderDataForScene(*this);

	m_tBottomUpAABBs.DeleteAllData();
	m_tBottomUpAABBs = ConstructBottomUpAABBBVHandRenderDataForScene(*this);

	m_tTopDownBoundingSpheres.DeleteAllData();
	m_tTopDownBoundingSpheres = ConstructTopDownBoundingSphereBVHandRenderDataForScene(*this);

	m_tBottomUpBoundingSpheres.DeleteAllData();
	m_tBottomUpBoundingSpheres = ConstructBottomUpBoundingSphereBVHandRenderDataForScene(*this);
}

void Visualization::UpdateAfterObjectPropertiesChange()
{
	// all updates and reset the sim
	CollisionDetection::UpdateBoundingVolumesForScene(*this);
	ReconstructAllTrees();
	ResetSimulation();
}

void Visualization::Update(float fDeltaTime)
{
	m_fDeltaTime = fDeltaTime;

	if (m_ePresentationMode == CONTINUOUS)
	{
		const float fPlaybackSpeedAdjustedDeltaTime = m_fDeltaTime * m_pPlaybackSpeeds[m_uiCurrentPlayBackSpeedIndex]; // at a playback speed of 0.25, time is 4 times slower for the simulation
		m_fAccumulatedTimeSinceLastUpdateStep += fPlaybackSpeedAdjustedDeltaTime;
		if (m_fAccumulatedTimeSinceLastUpdateStep >= 1.0f)
		{
			MoveToNextSimulationStep();
			m_fAccumulatedTimeSinceLastUpdateStep -= 1.0f;
		}
	}
}

void Visualization::Render2DGraph() const
{
	assert(glfwGetCurrentContext() == m_p2DGraphWindow->m_pGLFWwindow); // set the right context before calling this funtion

	if (m_p2DGraphWindow->IsMinimized())
		return;

	glClearColor(m_vec4fClearColor2DGraphWindow.r, m_vec4fClearColor2DGraphWindow.g, m_vec4fClearColor2DGraphWindow.b, m_vec4fClearColor2DGraphWindow.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/////////////////////////////////////////////////////////

	const std::vector<TreeNodeForRendering>* pvecNodeRenderData = nullptr;
	const std::vector<TreeNodeForRendering>* pvecLeafRenderData = nullptr;
	glm::vec4 vec4NodeRenderColor_Base;
	glm::vec4 vec4NodeRenderColor_Gradient;
	int16_t iDeepestDepthOfNodes = 0;
	if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::AABB)
	{
		// rendering the AABBs of tree nodes in the BVH

		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			pvecNodeRenderData = &m_tTopDownAABBs.m_vecTreeNodeDataForRendering;
			pvecLeafRenderData = &m_tTopDownAABBs.m_vecTreeLeafDataForRendering;
			vec4NodeRenderColor_Base = m_vec4TopDownNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4TopDownNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tTopDownAABBs.m_tBVH.m_iTDeepestDepthOfNodes;
		}
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::BOTTOMUP)
		{
			pvecNodeRenderData = &m_tBottomUpAABBs.m_vecTreeNodeDataForRendering;
			pvecLeafRenderData = &m_tBottomUpAABBs.m_vecTreeLeafDataForRendering;
			vec4NodeRenderColor_Base = m_vec4BottomUpNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4BottomUpNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tBottomUpAABBs.m_tBVH.m_iTDeepestDepthOfNodes;
		}
	}
	else if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::BOUNDING_SPHERE)
	{
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			pvecNodeRenderData = &m_tTopDownBoundingSpheres.m_vecTreeNodeDataForRendering;
			pvecLeafRenderData = &m_tTopDownBoundingSpheres.m_vecTreeLeafDataForRendering;
			vec4NodeRenderColor_Base = m_vec4TopDownNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4TopDownNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tTopDownBoundingSpheres.m_tBVH.m_iTDeepestDepthOfNodes;
		}
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::BOTTOMUP)
		{
			pvecNodeRenderData = &m_tBottomUpBoundingSpheres.m_vecTreeNodeDataForRendering;
			pvecLeafRenderData = &m_tBottomUpBoundingSpheres.m_vecTreeLeafDataForRendering;
			vec4NodeRenderColor_Base = m_vec4BottomUpNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4BottomUpNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tBottomUpBoundingSpheres.m_tBVH.m_iTDeepestDepthOfNodes;
		}
	}
	else
	{
		assert(!"disaster");
	}

	int16_t iAlreadyRenderedConstructionSteps = 0;
	for (const TreeNodeForRendering& rCurrentRendered2DNode : *pvecNodeRenderData)
	{
		bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRendered2DNode.m_iDepthInTree <= m_iMaximumRenderedTreeDepth);
		bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < m_iNumberStepsRendered);

		bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
		if (bShallRender)
		{
			glm::vec4 vec4RenderColor = vec4NodeRenderColor_Base;
			if (m_bNodeDepthColorGrading)
			{
				vec4RenderColor = InterpolateRenderColorForTreeNode(vec4NodeRenderColor_Base,
					vec4NodeRenderColor_Gradient,
					rCurrentRendered2DNode.m_iDepthInTree,
					iDeepestDepthOfNodes
				);
			}

			DrawNodeAtPosition(rCurrentRendered2DNode.m_vec2_2DNodeDrawPosition, vec4RenderColor);

			DrawLineFromTo(rCurrentRendered2DNode.m_vec2_2DLineToParentOrigin, rCurrentRendered2DNode.m_vec2_2DLineToParentTarget);
		}
		iAlreadyRenderedConstructionSteps++;
	}

	glm::vec4 vec4LeafDrawColor(0.0f, 0.0f, 0.55f, 1.0f);
	if (m_iNumberStepsRendered > 0)
	{
		for (const TreeNodeForRendering& rCurrentRendered2DLeaf : *pvecLeafRenderData)
		{
			Draw2DObjectAtPosition(rCurrentRendered2DLeaf.m_vec2_2DNodeDrawPosition, vec4LeafDrawColor);
			DrawLineFromTo(rCurrentRendered2DLeaf.m_vec2_2DLineToParentOrigin, rCurrentRendered2DLeaf.m_vec2_2DLineToParentTarget);
		}
	}

	/////////////////////////////////////////////////////////

	glfwSwapBuffers(m_p2DGraphWindow->m_pGLFWwindow);
}

void Visualization::UpdateFrameConstants()
{
	m_mat4Camera = m_tCamera.GetViewMatrix();
}

void Visualization::UpdateProjectionMatrices()
{
	if (!m_rMainWindow.IsMinimized())
	{
		// perspective projection matrix for 3D window
		m_mat4PerspectiveProjection3DWindow = glm::perspective(glm::radians(m_tCamera.Zoom), static_cast<float>(m_rMainWindow.m_iWindowWidth) / static_cast<float>(m_rMainWindow.m_iWindowHeight), 0.1f, m_fRenderDistance);

		// orthographic projection matrix for 3D window
		m_mat4OrthographicProjection3DWindow = glm::ortho(0.0f, static_cast<float>(m_rMainWindow.m_iWindowWidth), 0.0f, static_cast<float>(m_rMainWindow.m_iWindowHeight), -0.1f, m_fRenderDistance); // todo: replace with 2d version: https://glm.g-truc.net/0.9.2/api/a00245.html#ga71777a3b1d4fe1729cccf6eda05c8127
	}

	if (!m_p2DGraphWindow->IsMinimized())
	{
		// orthographic projection matrix for 2D window
		m_mat4OrthographicProjection2DWindow = glm::ortho(0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowWidth), 0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight), -0.1f, 10.0f); // todo: replace with 2d version: https://glm.g-truc.net/0.9.2/api/a00245.html#ga71777a3b1d4fe1729cccf6eda05c8127
	}
}

void Visualization::Render3DVisualization()
{
	assert(glfwGetCurrentContext() == m_rMainWindow.m_pGLFWwindow); // set the right context before calling this funtion

	if (m_rMainWindow.IsMinimized()) // hot fix to stop crashes when minimizing the window. needs proper handling in the future: https://www.glfw.org/docs/3.3/window_guide.html
		return;

	glAssert();

	// start by updating the uniform buffer containing the camera and projection matrices
	glBindBuffer(GL_UNIFORM_BUFFER, m_uiCameraProjectionUBO);
		// camera
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_mat4Camera), glm::value_ptr(m_mat4Camera));
		// perspective projection
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m_mat4Camera), sizeof(m_mat4PerspectiveProjection3DWindow), glm::value_ptr(m_mat4PerspectiveProjection3DWindow));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glClearColor(m_vec4fClearColor3DSceneWindow.r, m_vec4fClearColor3DSceneWindow.g, m_vec4fClearColor3DSceneWindow.b, m_vec4fClearColor3DSceneWindow.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glAssert();

	RenderRealObjects();
	RenderDataStructureObjects();
	Render3DSceneConstants();
	RenderHUDComponents();
	RenderVisualizationGUI();
}

void Visualization::RenderVisualizationGUI()
{
	assert(glfwGetCurrentContext() == m_rMainWindow.m_pGLFWwindow); // set the right context before calling this funtion

	RenderSimControlPanel();

	if (m_bShowHelpWindow)
		RenderHelpWindow();

	if (m_bShowSimulationOptions)
		RenderSimOptions();

	if (m_bShowObjectCreationWindow)
		RenderObjectCreationWindow();

	if (m_bShowObjectPropertiesWindow)
		RenderObjectPropertiesWindow();
}

void Visualization::RenderRealObjects() const
{
	assert(glfwGetCurrentContext() == m_rMainWindow.m_pGLFWwindow); // set the right context before calling this funtion

	glAssert();
	m_tFlatTextureShader.use();
	glAssert();
	m_tFlatTextureShader.setInt("texture1", 0);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_uiObjectDiffuseTexture);

	for (const SceneObject& rCurrentSceneObject : m_vecObjects)
	{
		const SceneObject::Transform& rCurrentTransform = rCurrentSceneObject.m_tTransform;

		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
		// translation
		world = glm::translate(world, rCurrentTransform.m_vec3Position);
		// rotation
		world = glm::rotate(world, glm::radians(rCurrentTransform.m_tRotation.m_fAngle), rCurrentTransform.m_tRotation.m_vec3Axis);
		// scale
		world = glm::scale(world, rCurrentTransform.m_vec3Scale);
		m_tFlatTextureShader.setMat4("world", world);

		glAssert();

		// render
		if (rCurrentSceneObject.m_eType == SceneObject::eType::CUBE)
		{
			glBindVertexArray(m_uiTexturedCubeVAO);
			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Cube::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}
		else if (rCurrentSceneObject.m_eType == SceneObject::eType::SPHERE)
		{
			glBindVertexArray(m_uiTexturedSphereVAO);
			glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
		}
		else
		{
			assert(!"disaster :)");
		}

		glAssert();
	}
}

void Visualization::RenderHUDComponents() const
{
	// Crosshair
	{
		glAssert();
		const Shader& rCurrentShader = m_tHUDComponentColorShader;
		rCurrentShader.use();
		glAssert();
		rCurrentShader.setInt("transparencyMask", 0);
		glAssert();
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiCrosshairTexture);

		glBindVertexArray(m_uiTexturedPlaneVAO);

		// world matrix
		glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
		glm::vec3 vec3CrosshairTranslationVector(static_cast<float>(m_rMainWindow.m_iWindowWidth) * 0.5f, static_cast<float>(m_rMainWindow.m_iWindowHeight) * 0.5f, 0.0f); // in the middle of the window
		mat4World = glm::translate(mat4World, vec3CrosshairTranslationVector);
		mat4World = glm::scale(mat4World, glm::vec3(m_fCrossHairScaling, m_fCrossHairScaling, 1.0f));
		mat4World = glm::rotate(mat4World, glm::pi<float>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)); // default plane/quad is defined as lying face up flat on the floor. this makes it "stand up" and face the camera. TODO: use "HUD" plane that is facing the camra
		rCurrentShader.setMat4("world", mat4World);

		// no camera matrix for hud components!

		// projection matrix
		rCurrentShader.setMat4("orthoProjection", m_mat4OrthographicProjection3DWindow);

		// setting the color
		rCurrentShader.setVec4("color", m_vec4CrossHairColor);

		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

		glAssert();
	}
}

void Visualization::RenderDataStructureObjects() const
{
	const Shader& rCurrentShader = m_tColorShader;
	rCurrentShader.use();
	glAssert();

	glDisable(GL_CULL_FACE);

	// AABBs
	if (m_bRenderObjectAABBs)
	{
		// render colour yellow for AABBs
		glm::vec4 vec4AABBRenderColor(1.0f, 1.0f, 0.0f, 1.0f);
		rCurrentShader.setVec4("color", m_vec4AABBColor);

		for (const SceneObject& rCurrentSceneObject : m_vecObjects)
		{
			RenderAABBOfSceneObject(rCurrentSceneObject, rCurrentShader);
		}
	}

	// Bounding Spheres
	if (m_bRenderObjectBoundingSpheres)
	{
		rCurrentShader.setVec4("color", m_vec4BoundingSphereColor);

		for (const SceneObject& rCurrentSceneObject : m_vecObjects)
		{
			RenderBoundingSphereOfSceneObject(rCurrentSceneObject, rCurrentShader);
		}
	}

	
	const std::vector<TreeNodeForRendering>* pvecNodeRenderData = nullptr;
	glm::vec4 vec4NodeRenderColor_Base;
	glm::vec4 vec4NodeRenderColor_Gradient;
	int16_t iDeepestDepthOfNodes = 0;
	if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::AABB)
	{
		// rendering the AABBs of tree nodes in the BVH

		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			pvecNodeRenderData = &m_tTopDownAABBs.m_vecTreeNodeDataForRendering;
			vec4NodeRenderColor_Base = m_vec4TopDownNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4TopDownNodeRenderColor_Gradient;
			iDeepestDepthOfNodes =  m_tTopDownAABBs.m_tBVH.m_iTDeepestDepthOfNodes;
		}
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::BOTTOMUP)
		{
			pvecNodeRenderData = &m_tBottomUpAABBs.m_vecTreeNodeDataForRendering;
			vec4NodeRenderColor_Base = m_vec4BottomUpNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4BottomUpNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tBottomUpAABBs.m_tBVH.m_iTDeepestDepthOfNodes;
		}
	}
	else if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::BOUNDING_SPHERE)
	{
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::TOPDOWN)
		{
			pvecNodeRenderData = &m_tTopDownBoundingSpheres.m_vecTreeNodeDataForRendering;
			vec4NodeRenderColor_Base = m_vec4TopDownNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4TopDownNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tTopDownBoundingSpheres.m_tBVH.m_iTDeepestDepthOfNodes;
		}
		if (GetCurrenBVHConstructionStrategy() == Visualization::eBVHConstructionStrategy::BOTTOMUP)
		{
			pvecNodeRenderData = &m_tBottomUpBoundingSpheres.m_vecTreeNodeDataForRendering;
			vec4NodeRenderColor_Base = m_vec4BottomUpNodeRenderColor;
			vec4NodeRenderColor_Gradient = m_vec4BottomUpNodeRenderColor_Gradient;
			iDeepestDepthOfNodes = m_tBottomUpBoundingSpheres.m_tBVH.m_iTDeepestDepthOfNodes;
		}
	}
	else
	{
		assert(!"disaster");
	}

	int16_t iAlreadyRenderedConstructionSteps = 0;
	for (const TreeNodeForRendering& rCurrentRenderedBVHBoundingVolume : *pvecNodeRenderData)
	{
		bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRenderedBVHBoundingVolume.m_iDepthInTree <= m_iMaximumRenderedTreeDepth);
		bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < m_iNumberStepsRendered);

		bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
		if (bShallRender)
		{
			glm::vec4 vec4RenderColor = vec4NodeRenderColor_Base;
			if (m_bNodeDepthColorGrading)
			{
				vec4RenderColor = InterpolateRenderColorForTreeNode(vec4NodeRenderColor_Base,
					vec4NodeRenderColor_Gradient,
					rCurrentRenderedBVHBoundingVolume.m_iDepthInTree,
					iDeepestDepthOfNodes
				);
			}

			rCurrentShader.setVec4("color", vec4RenderColor);
			
			if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::AABB)
				RenderTreeNodeAABB(rCurrentRenderedBVHBoundingVolume, rCurrentShader);
			if (GetCurrentBVHBoundingVolume() == Visualization::eBVHBoundingVolume::BOUNDING_SPHERE)
				RenderTreeNodeBoundingsphere(rCurrentRenderedBVHBoundingVolume, rCurrentShader);
		}
		iAlreadyRenderedConstructionSteps++;
	}

	glEnable(GL_CULL_FACE);
}

void Visualization::MouseMoveCallback(GLFWwindow* pWindow, double dXPosition, double dYPosition)
{
	assert(m_p2DGraphWindow);

	if (pWindow == m_rMainWindow.m_pGLFWwindow) // in the main window
	{
		const float fXPosition = static_cast<float>(dXPosition);
		const float fYPosition = static_cast<float>(dYPosition);

		if (m_rMainWindow.IsMouseCaptured()) // Control the camera only when mouse is captured
		{
			if (m_rMainWindow.m_bFirstMouse)
			{
				m_rMainWindow.m_fLastXOfMouse = fXPosition;
				m_rMainWindow.m_fLastYOfMouse = fYPosition;
				m_rMainWindow.m_bFirstMouse = false;
			}

			float xoffset = fXPosition - m_rMainWindow.m_fLastXOfMouse;
			float yoffset = m_rMainWindow.m_fLastYOfMouse - fYPosition; // reversed since y-coordinates go from bottom to top

			m_rMainWindow.m_fLastXOfMouse = fXPosition;
			m_rMainWindow.m_fLastYOfMouse = fYPosition;

			
			m_tCamera.ProcessMouseMovement(xoffset, yoffset);
		}
		else	// reacting to mouse movement when the camera is not active
		{
			// empty
		}
	}
	else if(pWindow == m_p2DGraphWindow->m_pGLFWwindow) // in the 2d graph window
	{
		// empty now
	}
	else
	{
		assert(!"it's a disastah");
	}
}

void Visualization::MouseClickCallback(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	assert(m_p2DGraphWindow);

	if (pWindow == m_rMainWindow.m_pGLFWwindow) // in the main window
	{
		if (iButton == GLFW_MOUSE_BUTTON_LEFT && iAction == GLFW_PRESS) // single click of left mouse button
		{
			
			if (m_rMainWindow.IsMouseCaptured())
			{
				// reacting to clicks made while camera control is active
				CrosshairClick();
			}
			else
			{
				// reacting to clicks of a freely moving cursor
				CursorClick();	
			}
		}
	}
	else if (pWindow == m_p2DGraphWindow->m_pGLFWwindow) // in the 2d graph window
	{
		// empty now
	}
	else
	{
		assert(!"it's a disastah");
	}
}

void Visualization::WindowResizeCallBack(GLFWwindow * pWindow, int iNewWidth, int iNewHeight)
{
	if (pWindow == m_p2DGraphWindow->m_pGLFWwindow)
	{
		m_p2DGraphWindow->m_iWindowWidth = iNewWidth;
		m_p2DGraphWindow->m_iWindowHeight = iNewHeight;
		m_p2DGraphWindow->SetAsCurrentRenderContext();
		glViewport(0, 0, iNewWidth, iNewHeight);
	}
}

void Visualization::ProcessKeyboardInput()
{
	// continuous inputs
	{
		// camera control
		{

			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_W) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(FORWARD, m_fDeltaTime);
			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_S) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(BACKWARD, m_fDeltaTime);
			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_A) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(LEFT, m_fDeltaTime);
			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_D) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(RIGHT, m_fDeltaTime);

			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_Q) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(UP, m_fDeltaTime);
			if (glfwGetKey(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_E) == GLFW_PRESS)
				m_tCamera.ProcessKeyboard(DOWN, m_fDeltaTime);
		}
	}

	// discrete inputs
	{
		if (Engine::IsDiscreteKeyReadyForWindow(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_H))
		{
			ToggleHelpWindow();
		}

		if (Engine::IsDiscreteKeyReadyForWindow(m_rMainWindow.m_pGLFWwindow, GLFW_KEY_M))
		{
			m_rMainWindow.SetHardCaptureMouse(!m_rMainWindow.IsMouseCaptured());	// toggle between captured mouse or a cursor
			m_bGUICaptureMouse = !m_rMainWindow.IsMouseCaptured();	// control GUI behaviour
		}
	}
}

void Visualization::ResetSimulation()
{
	m_iNumberStepsRendered = 0;
	m_iMaximumRenderedTreeDepth = 100;
	m_iSimulationDirectionSign = 1;
	m_ePresentationMode = DISCRETE;
	m_uiCurrentPlayBackSpeedIndex = 2u;
}

void Visualization::PlaySimulation()
{
	m_ePresentationMode = CONTINUOUS;
	m_fAccumulatedTimeSinceLastUpdateStep = 0.0f;
}

void Visualization::PauseSimulation()
{
	m_ePresentationMode = DISCRETE;
}

void Visualization::IncreaseSimulationSpeed()
{
	assert(m_uiCurrentPlayBackSpeedIndex <= 4u);
	if (m_uiCurrentPlayBackSpeedIndex < 4u)
		m_uiCurrentPlayBackSpeedIndex++;
}

void Visualization::DecreaseSimulationSpeed()
{
	assert(m_uiCurrentPlayBackSpeedIndex <= 4u);
	if (m_uiCurrentPlayBackSpeedIndex > 0u)
		m_uiCurrentPlayBackSpeedIndex--;
}

float Visualization::GetCurrentSimulationSpeed() const
{
	return m_pPlaybackSpeeds[m_uiCurrentPlayBackSpeedIndex];
}

void Visualization::InvertSimulationProgression()
{
	m_iSimulationDirectionSign *= -1;
}

void Visualization::MoveToNextSimulationStep()
{
	const int iNextNumberOfConstructionStepsRendered = m_iNumberStepsRendered + m_iSimulationDirectionSign;

	// bounds checks
	m_iNumberStepsRendered = std::max<int>(0, iNextNumberOfConstructionStepsRendered);
	assert(m_tTopDownAABBs.m_vecTreeNodeDataForRendering.size() <= std::numeric_limits<int>::max());	// make sure that number fits or chaos might ensue. This assertion will probably never fire... but it doesnt hurt either
	m_iNumberStepsRendered = std::min<int>(m_iNumberStepsRendered, static_cast<int>(m_tTopDownAABBs.m_vecTreeNodeDataForRendering.size())); // todo: rework to dynamically work with current set BVH
}

Visualization::eBVHConstructionStrategy Visualization::GetCurrenBVHConstructionStrategy() const
{
	return m_eConstructionStrategy;
}

void Visualization::SetNewBVHConstructionStrategy(eBVHConstructionStrategy eNewStrategy)
{
	if (m_eConstructionStrategy != eNewStrategy)
	{
		m_eConstructionStrategy = eNewStrategy;
		ResetSimulation();
	}
}

Visualization::eBVHBoundingVolume Visualization::GetCurrentBVHBoundingVolume() const
{
	return m_eBVHBoundingVolume;
}

void Visualization::SetNewBVHBoundingVolume(eBVHBoundingVolume eNewBoundingVolume)
{
	m_eBVHBoundingVolume = eNewBoundingVolume;
}

void Visualization::DeleteGivenObject(SceneObject* pToBeDeletedObject)
{
	assert(pToBeDeletedObject);

	for (auto it = m_vecObjects.begin(); it != m_vecObjects.end(); it++)
	{
		if (&(*it) == pToBeDeletedObject)
		{
			m_vecObjects.erase(it);
			break;
		}
	}

	// all updates and reset the sim
	//CollisionDetection::UpdateBoundingVolumesForScene(*this);
	ResetSimulation();
	if(!m_vecObjects.empty())
		ReconstructAllTrees();
}

void Visualization::AddNewSceneObject(SceneObject & rNewSceneObject)
{
	m_vecObjects.push_back(rNewSceneObject);

	// updating the data structures
	CollisionDetection::ConstructBoundingVolumesForScene(*this);
	CollisionDetection::UpdateBoundingVolumesForScene(*this);
	ReconstructAllTrees();
	ResetSimulation();
}

void Visualization::ClearCurrentScene()
{
	m_vecObjects.clear();
	ResetSimulation();

	m_tTopDownAABBs.DeleteAllData();
	m_tBottomUpAABBs.DeleteAllData();
	m_tTopDownBoundingSpheres.DeleteAllData();
	m_tBottomUpBoundingSpheres.DeleteAllData();
}

void Visualization::InitPlaybackSpeeds()
{
	m_pPlaybackSpeeds[0] = 0.25f;
	m_pPlaybackSpeeds[1] = 0.50f;
	m_pPlaybackSpeeds[2] = 1.00f;
	m_pPlaybackSpeeds[3] = 2.00f;
	m_pPlaybackSpeeds[4] = 4.00f;
}

void Visualization::InitRenderColors()
{
	// clear colors
	m_vec4fClearColor3DSceneWindow = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
	m_vec4fClearColor2DGraphWindow = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

	// grid
	m_vec4GridColorX = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4GridColorY = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); // cyan
	m_vec4GridColorZ = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple

	// crosshair
	m_vec4CrossHairColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // green

	// bounding volumes
	m_vec4AABBColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4BoundingSphereColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // blue

	// node colors
	m_vec4TopDownNodeRenderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
	m_vec4TopDownNodeRenderColor_Gradient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // black
	m_vec4BottomUpNodeRenderColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple
	m_vec4BottomUpNodeRenderColor_Gradient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // black
}

void Visualization::RecursiveTopDownTree_AABB(CollisionDetection::BVHTreeNode ** pTree, SceneObject * pSceneObjects, size_t uiNumSceneObjects)
{
	assert(pTree);
	assert(pSceneObjects);
	assert(uiNumSceneObjects > 0);

	const uint8_t uiNumberOfObjectsPerLeaf = 1u;
	CollisionDetection::BVHTreeNode* pNewNode = new CollisionDetection::BVHTreeNode;
	*pTree = pNewNode;

	if (uiNumSceneObjects <= uiNumberOfObjectsPerLeaf) // is a leaf
	{
		assert(uiNumSceneObjects == 1); // needs reconsideration for >1 objects per leaf
		// bounding volumes for single objects is already done, no need to compute that here
		pNewNode->m_uiNumOjbects = static_cast<uint8_t>(uiNumSceneObjects);
		pNewNode->m_pObjects = pSceneObjects;
	}
	else // is a node
	{
		// create AABB bounding volume for the current set of objects
		pNewNode->m_tAABBForNode = CollisionDetection::CreateAABBForMultipleObjects(pSceneObjects, uiNumSceneObjects);

		// partition current set into subsets IN PLACE!!!
		size_t uiNumLeftchildren = CollisionDetection::PartitionSceneObjectsInPlace_AABB(pSceneObjects, uiNumSceneObjects);

		// move on with "left" side
		RecursiveTopDownTree_AABB(&(pNewNode->m_pLeft), pSceneObjects, uiNumLeftchildren);

		// move on with "right" side
		RecursiveTopDownTree_AABB(&(pNewNode->m_pRight), pSceneObjects + uiNumLeftchildren, uiNumSceneObjects - uiNumLeftchildren);
	}
}

CollisionDetection::BVHTreeNode * Visualization::BottomUpTree_AABB(SceneObject * pSceneObjects, size_t uiNumSceneObjects, BVHRenderingDataTuple& rBVHRenderDataTuple)
{
	assert(uiNumSceneObjects > 0);

	CollisionDetection::BVHTreeNode** pTempNodes = new CollisionDetection::BVHTreeNode*[uiNumSceneObjects]; // careful: these are pointers to pointers

	// creating all leaf nodes: number leaves == number objects
	for (size_t uiCurrentNewLeafNode = 0u; uiCurrentNewLeafNode < uiNumSceneObjects; uiCurrentNewLeafNode++)
	{
		pTempNodes[uiCurrentNewLeafNode] = new CollisionDetection::BVHTreeNode;	// assigning the adress of the new leaf node to the pointer pointed at by pTempNodes[current]
		pTempNodes[uiCurrentNewLeafNode]->m_uiNumOjbects = 1u;
		pTempNodes[uiCurrentNewLeafNode]->m_pObjects = &pSceneObjects[uiCurrentNewLeafNode];
		pTempNodes[uiCurrentNewLeafNode]->m_tAABBForNode = pTempNodes[uiCurrentNewLeafNode]->m_pObjects->m_tWorldSpaceAABB;
	}

	// for visualization purposes
	int16_t iNumConstructedNodes = 0;

	// merging leaves into nodes until root node is constructed
	while (uiNumSceneObjects > 1) {
		// Pick two volumes to pair together
		size_t uiMergedNodeIndex1 = 0, uiMergedNodeIndex2 = 0;
		CollisionDetection::FindBottomUpNodesToMerge_AABB(pTempNodes, uiNumSceneObjects, uiMergedNodeIndex1, uiMergedNodeIndex2);

		// Pair them in new parent node
		CollisionDetection::BVHTreeNode* pParentNode = new CollisionDetection::BVHTreeNode;
		pParentNode->m_pLeft = pTempNodes[uiMergedNodeIndex1];
		pParentNode->m_pRight = pTempNodes[uiMergedNodeIndex2];
		// construct AABB for that parent node (adaption from orginal code)
		pParentNode->m_tAABBForNode = CollisionDetection::MergeTwoAABBs(pTempNodes[uiMergedNodeIndex1]->m_tAABBForNode, pTempNodes[uiMergedNodeIndex2]->m_tAABBForNode);

		// for visualization/rendering purposes
		TreeNodeForRendering tNewAABBNodeForRendering;
		tNewAABBNodeForRendering.m_iRenderingOrder = iNumConstructedNodes++;
		tNewAABBNodeForRendering.m_pNodeToBeRendered = pParentNode;
		rBVHRenderDataTuple.m_vecTreeNodeDataForRendering.push_back(tNewAABBNodeForRendering);

		//Updating the current set of nodes accordingly
		size_t uiMinIndex = uiMergedNodeIndex1, uiMaxIndex = uiMergedNodeIndex2;
		if (uiMergedNodeIndex1 > uiMergedNodeIndex2)
		{
			uiMinIndex = uiMergedNodeIndex2;
			uiMaxIndex = uiMergedNodeIndex1;
		}
		pTempNodes[uiMinIndex] = pParentNode;
		pTempNodes[uiMaxIndex] = pTempNodes[uiNumSceneObjects - 1];
		uiNumSceneObjects--;
	}

	CollisionDetection::BVHTreeNode* pRootNode = pTempNodes[0]; // careful: getting the pointer to root by dereferencing the pointer to pointer
	delete[] pTempNodes;
	return pRootNode;
}

void Visualization::RecursiveTopDownTree_BoundingSphere(CollisionDetection::BVHTreeNode ** pNode, SceneObject * pSceneObjects, size_t uiNumSceneObjects)
{
	assert(pNode);
	assert(pSceneObjects);
	assert(uiNumSceneObjects > 0);

	const uint8_t uiNumberOfObjectsPerLeaf = 1u;
	CollisionDetection::BVHTreeNode* pNewNode = new CollisionDetection::BVHTreeNode;
	*pNode = pNewNode;

	if (uiNumSceneObjects <= uiNumberOfObjectsPerLeaf) // is a leaf
	{
		assert(uiNumSceneObjects == 1); // needs reconsideration for >1 objects per leaf
		// bounding volumes for single objects is already done, no need to compute that here
		pNewNode->m_uiNumOjbects = static_cast<uint8_t>(uiNumSceneObjects);
		pNewNode->m_pObjects = pSceneObjects;
	}
	else // is a node
	{
		// create Bounding Sphere volume for the current set of objects
		pNewNode->m_tBoundingSphereForNode = CollisionDetection::CreateBoundingSphereForMultipleObjects(pSceneObjects, uiNumSceneObjects);

		// partition current set into subsets IN PLACE!!!
		size_t uiPartitioningIndex = CollisionDetection::PartitionSceneObjectsInPlace_BoundingSphere(pSceneObjects, uiNumSceneObjects);

		// move on with "left" side
		RecursiveTopDownTree_BoundingSphere(&(pNewNode->m_pLeft), pSceneObjects, uiPartitioningIndex);

		// move on with "right" side
		RecursiveTopDownTree_BoundingSphere(&(pNewNode->m_pRight), pSceneObjects + uiPartitioningIndex, uiNumSceneObjects - uiPartitioningIndex);
	}
}

CollisionDetection::BVHTreeNode * Visualization::BottomUpTree_BoundingSphere(SceneObject * pSceneObjects, size_t uiNumSceneObjects, BVHRenderingDataTuple& rBVHRenderDataTuple)
{
	assert(uiNumSceneObjects > 0);

	CollisionDetection::BVHTreeNode** pTempNodes = new CollisionDetection::BVHTreeNode*[uiNumSceneObjects]; // careful: these are pointers to pointers

	// creating all leaf nodes: number leaves == number objects
	for (size_t uiCurrentNewLeafNode = 0u; uiCurrentNewLeafNode < uiNumSceneObjects; uiCurrentNewLeafNode++)
	{
		pTempNodes[uiCurrentNewLeafNode] = new CollisionDetection::BVHTreeNode;	// assigning the adress of the new leaf node to the pointer pointed at by pTempNodes[current]
		pTempNodes[uiCurrentNewLeafNode]->m_uiNumOjbects = 1u;
		pTempNodes[uiCurrentNewLeafNode]->m_pObjects = &pSceneObjects[uiCurrentNewLeafNode];
		pTempNodes[uiCurrentNewLeafNode]->m_tBoundingSphereForNode = pTempNodes[uiCurrentNewLeafNode]->m_pObjects->m_tWorldSpaceBoundingSphere;
	}

	// for visualization purposes
	int16_t iNumConstructedNodes = 0;

	// merging leaves into nodes until root node is constructed
	while (uiNumSceneObjects > 1) {
		// Pick two volumes to pair together
		size_t uiMergedNodeIndex1 = 0, uiMergedNodeIndex2 = 0;
		CollisionDetection::FindBottomUpNodesToMerge_BoundingSphere(pTempNodes, uiNumSceneObjects, uiMergedNodeIndex1, uiMergedNodeIndex2);

		// Pair them in new parent node
		CollisionDetection::BVHTreeNode* pParentNode = new CollisionDetection::BVHTreeNode;
		pParentNode->m_pLeft = pTempNodes[uiMergedNodeIndex1];
		pParentNode->m_pRight = pTempNodes[uiMergedNodeIndex2];
		// construct Bounding Sphere for that parent node (adaption from orginal code)
		pParentNode->m_tBoundingSphereForNode = CollisionDetection::MergeTwoBoundingSpheres(pTempNodes[uiMergedNodeIndex1]->m_tBoundingSphereForNode, pTempNodes[uiMergedNodeIndex2]->m_tBoundingSphereForNode);

		// for visualization/rendering purposes
		TreeNodeForRendering tNewBoundingSphereNodeForRendering;
		tNewBoundingSphereNodeForRendering.m_iRenderingOrder = iNumConstructedNodes++;
		tNewBoundingSphereNodeForRendering.m_pNodeToBeRendered = pParentNode;
		rBVHRenderDataTuple.m_vecTreeNodeDataForRendering.push_back(tNewBoundingSphereNodeForRendering);

		//Updating the current set of nodes accordingly
		size_t uiMinIndex = uiMergedNodeIndex1, uiMaxIndex = uiMergedNodeIndex2;
		if (uiMergedNodeIndex1 > uiMergedNodeIndex2)
		{
			uiMinIndex = uiMergedNodeIndex2;
			uiMaxIndex = uiMergedNodeIndex1;
		}
		pTempNodes[uiMinIndex] = pParentNode;
		pTempNodes[uiMaxIndex] = pTempNodes[uiNumSceneObjects - 1];
		uiNumSceneObjects--;
	}

	CollisionDetection::BVHTreeNode* pRootNode = pTempNodes[0]; // careful: getting the pointer to root by dereferencing the pointer to pointer
	delete[] pTempNodes;
	return pRootNode;
}

void Visualization::Render3DSceneConstants() const
{
	// uniform grid
	{
		glAssert();

		// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
		// -------------------------------------------------------------------------------------------
		const Shader& rCurrentShader = m_tMaskedColorShader;
		rCurrentShader.use();
		glAssert();
		rCurrentShader.setInt("transparencyMask", 0);

		glAssert();

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_uiGridMaskTexture);

		glAssert();

		glDisable(GL_CULL_FACE);

		glBindVertexArray(m_uiGridPlaneVAO);

		if (m_bRenderGridXPlane)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 mat4WorldXPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldXPlane = glm::translate(mat4WorldXPlane, glm::vec3(m_vec3GridPositionsOnAxes.x, 0.0f, 0.0f));
			mat4WorldXPlane = glm::rotate(mat4WorldXPlane, glm::pi<float>() * 0.5f, glm::vec3(0.0f, 0.0f, 1.0f));
			rCurrentShader.setMat4("world", mat4WorldXPlane);

			// setting the color
			rCurrentShader.setVec4("color", m_vec4GridColorX);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}

		if (m_bRenderGridYPlane)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 mat4WorldYPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldYPlane = glm::translate(mat4WorldYPlane, glm::vec3(0.0f, m_vec3GridPositionsOnAxes.y, 0.0f));
			rCurrentShader.setMat4("world", mat4WorldYPlane);
			// no rotation needed since the default rendered plane is defined as lying flat on the "ground", facing upwards

			// setting the color			
			rCurrentShader.setVec4("color", m_vec4GridColorY);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}

		if (m_bRenderGridZPlane)
		{
			glm::mat4 mat4WorldZPlane = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			mat4WorldZPlane = glm::translate(mat4WorldZPlane, glm::vec3(0.0f, 0.0f, -m_vec3GridPositionsOnAxes.z));
			mat4WorldZPlane = glm::rotate(mat4WorldZPlane, glm::pi<float>() * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f));
			rCurrentShader.setMat4("world", mat4WorldZPlane);

			// setting the color
			rCurrentShader.setVec4("color", m_vec4GridColorZ);

			glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
		}

		glEnable(GL_CULL_FACE);

		glAssert();
	}
}

void Visualization::RenderTreeNodeAABB(const TreeNodeForRendering & rTreeNodeAABB, const Shader & rShader) const
{
	// the AABB
	const CollisionDetection::AABB& rRenderedAABB = rTreeNodeAABB.m_pNodeToBeRendered->m_tAABBForNode;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedAABB.m_vec3Center);
	// scale
	const float fDetaultCubeHalfWidth = Primitives::Cube::DefaultCubeHalfWidth;
	world = glm::scale(world, rRenderedAABB.m_vec3Radius / glm::vec3(fDetaultCubeHalfWidth, fDetaultCubeHalfWidth, fDetaultCubeHalfWidth)); // scaling a "default" cube so it has the same extents as the current AABB

	rShader.setMat4("world", world);

	glAssert();

	// render the cube
	glBindVertexArray(m_uiColoredCubeVAO);
	glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glAssert();
}

void Visualization::RenderTreeNodeBoundingsphere(const TreeNodeForRendering & rTreeNodeAABB, const Shader & rShader) const
{
	// the bounding sphere
	const CollisionDetection::BoundingSphere& rRenderedBoundingSphere = rTreeNodeAABB.m_pNodeToBeRendered->m_tBoundingSphereForNode;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedBoundingSphere.m_vec3Center);
	// scale
	const float fDefaultSphereRadius = Primitives::Sphere::SphereDefaultRadius;
	const float fRenderedSphereRadius = rRenderedBoundingSphere.m_fRadius / fDefaultSphereRadius;
	world = glm::scale(world, glm::vec3(fRenderedSphereRadius, fRenderedSphereRadius, fRenderedSphereRadius)); // scaling a "default" sphere so it has the same extents as the current AABB

	rShader.setMat4("world", world);

	glAssert();

	// render the sphere
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(m_uiTexturedSphereVAO);
	glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glAssert();
}

void Visualization::RenderAABBOfSceneObject(const SceneObject & rSceneObject, const Shader & rShader) const
{
	// the AABBs
	const CollisionDetection::AABB& rRenderedAABB = rSceneObject.m_tWorldSpaceAABB;
	const CollisionDetection::AABB& rLocalSpaceAABBReference = rSceneObject.m_tLocalSpaceAABB;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedAABB.m_vec3Center);
	// scale
	world = glm::scale(world, rRenderedAABB.m_vec3Radius / rLocalSpaceAABBReference.m_vec3Radius);

	rShader.setMat4("world", world);

	glAssert();

	// render the object appropriately
	glBindVertexArray(m_uiColoredCubeVAO);
	glDrawElements(GL_LINE_STRIP, static_cast<GLsizei>(sizeof(Primitives::Cube::SimpleIndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);

	glAssert();
}

void Visualization::RenderBoundingSphereOfSceneObject(const SceneObject & rSceneObject, const Shader & rShader) const
{
	const CollisionDetection::BoundingSphere& rRenderedBoundingSphere = rSceneObject.m_tWorldSpaceBoundingSphere;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation
	world = glm::translate(world, rRenderedBoundingSphere.m_vec3Center);
	// scale 
	const float fScale = rRenderedBoundingSphere.m_fRadius / Primitives::Sphere::SphereDefaultRadius;
	world = glm::scale(world, glm::vec3(fScale, fScale, fScale));

	rShader.setMat4("world", world);

	glAssert();

	// render a sphere
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(m_uiTexturedSphereVAO);
	glDrawArrays(GL_TRIANGLES, 0, Primitives::Sphere::NumberOfTrianglesInSphere * 3);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glAssert();
}

void Visualization::FreeGPUResources()
{
	// todo: there are resources missing here

	// Vertex Buffers and Vertey Arrays
	glDeleteVertexArrays(1, &m_uiTexturedCubeVAO);
	glDeleteBuffers(1, &m_uiTexturedCubeVBO);
	glDeleteBuffers(1, &m_uiTexturedCubeEBO);

	glDeleteVertexArrays(1, &m_uiColoredCubeVAO);
	glDeleteBuffers(1, &m_uiColoredCubeVBO);
	glDeleteBuffers(1, &m_uiColoredCubeEBO);

	glDeleteVertexArrays(1, &m_uiTexturedPlaneVAO);
	glDeleteBuffers(1, &m_uiTexturedPlaneVBO);
	glDeleteBuffers(1, &m_uiTexturedPlaneEBO);

	glDeleteVertexArrays(1, &m_uiColoredPlaneVAO);
	glDeleteBuffers(1, &m_uiColoredPlaneVBO);
	glDeleteBuffers(1, &m_uiColoredPlaneEBO);

	glDeleteVertexArrays(1, &m_uiTexturedSphereVAO);
	glDeleteBuffers(1, &m_uiTexturedSphereVBO);
	//glDeleteBuffers(1, &m_uiTexturedSphereEBO);

	// Uniform Buffers
	glDeleteBuffers(1, &m_uiCameraProjectionUBO);

	// Textures
	glDeleteTextures(1, &m_uiObjectDiffuseTexture);
	glDeleteTextures(1, &m_uiGridMaskTexture);
}

glm::vec4 Visualization::InterpolateRenderColorForTreeNode(const glm::vec4 & rColor1, const glm::vec4 & rColor2, int16_t iDepthInTree, int16_t iDeepestDepthOfNodes) const
{
	assert(iDepthInTree >= 0);

	if (iDeepestDepthOfNodes == 0)
		return rColor1;

	const float fWeightPerDepthLevel = 1.0f / static_cast<float>(iDeepestDepthOfNodes); // OK
	const float fWeightColor1 = 1.0f - fWeightPerDepthLevel * static_cast<float>(iDepthInTree);
	const float fWeightColor2 = 1.0f - fWeightColor1;

	return rColor1 * fWeightColor1 + rColor2 * fWeightColor2;
}

void Visualization::TraverseTreeForDataForTopDownRendering_AABB(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple & rBVHRenderDataTuple, int16_t iDepthInTree)
{
	assert(pNode);

	if (pNode->IsANode()) // node -> not a leaf
	{
		// if it is a node, there was a partitioning step, which means there have to be two children
		assert(pNode->m_pLeft);
		assert(pNode->m_pRight);

		// save relevant data for rendering
		TreeNodeForRendering tNewAABBForRendering;
		tNewAABBForRendering.m_iDepthInTree = iDepthInTree;
		tNewAABBForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeNodeDataForRendering.push_back(tNewAABBForRendering);

		rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes = std::max(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes, iDepthInTree);
		iDepthInTree++; // we are now one level deeper

		// traverse left ...
		TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pLeft, rBVHRenderDataTuple, iDepthInTree);
		// ... then right
		TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pRight, rBVHRenderDataTuple, iDepthInTree);
	}
	else // is a leaf
	{
		TreeNodeForRendering tNewLeafNodeForRendering;
		tNewLeafNodeForRendering.m_iDepthInTree = iDepthInTree;
		tNewLeafNodeForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeLeafDataForRendering.push_back(tNewLeafNodeForRendering);
	}
}

void Visualization::TraverseTreeForDataForBottomUpRendering_AABB(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple & rBVHRenderDataTuple, int16_t iDepthInTree)
{
	/*
		This is going to be ugly.
		We are facing a problem here: For Bottom up construction of a BVH, the order in which AABBs are constructed is not the same as the tree is later being traversed.
		That means, there is no information in the tree, in which order its nodes were constructed.
		Also, i want to keep this information outside of the tree, since it is really only relevant for rendering/visualizing.
		So, in an effort to keep the visualization only data out of the tree, we have to now do something very ugly.
		Order of construction can only be determined during construction. Tree depth of a given node and its associated AABB can only be determined during traversal.
	*/

	assert(pNode);

	if (pNode->IsANode()) // node -> not a leaf
	{
		// if it is a node, there was a partitioning step, which means there have to be two children
		assert(pNode->m_pLeft);
		assert(pNode->m_pRight);

		// the ugly part. finding the appropriate rendering object to assign it its true tree depth.
		for (TreeNodeForRendering& rCurrentRenderObject : rBVHRenderDataTuple.m_vecTreeNodeDataForRendering)
		{
			if (pNode == rCurrentRenderObject.m_pNodeToBeRendered)
			{
				rCurrentRenderObject.m_iDepthInTree = iDepthInTree;
				break;
			}
		}

		rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes= std::max(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes, iDepthInTree);
		iDepthInTree++; // we are now one level deeper				

		// traverse left ...
		TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pLeft, rBVHRenderDataTuple, iDepthInTree);
		// ... then right
		TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pRight, rBVHRenderDataTuple, iDepthInTree);
	}
	else // is a leaf
	{
		TreeNodeForRendering tNewLeafNodeForRendering;
		tNewLeafNodeForRendering.m_iDepthInTree = iDepthInTree;
		tNewLeafNodeForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeLeafDataForRendering.push_back(tNewLeafNodeForRendering);
	}
}

void Visualization::TraverseTreeForDataForTopDownRendering_BoundingSphere(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple & rBVHRenderDataTuple, int16_t iDepthInTree)
{
	assert(pNode);

	if (pNode->IsANode()) // node -> is a leaf
	{
		// if it is a node, there was a partitioning step, which means there have to be two children
		assert(pNode->m_pLeft);
		assert(pNode->m_pRight);

		// save relevant data for rendering
		TreeNodeForRendering tNewBoundingSphereForRendering;
		tNewBoundingSphereForRendering.m_iDepthInTree = iDepthInTree;
		tNewBoundingSphereForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeNodeDataForRendering.push_back(tNewBoundingSphereForRendering);

		rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes = std::max(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes, iDepthInTree);
		iDepthInTree++; // we are now one level deeper

		// traverse left ...
		TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pLeft, rBVHRenderDataTuple, iDepthInTree);
		// ... then right
		TraverseTreeForDataForTopDownRendering_AABB(pNode->m_pRight, rBVHRenderDataTuple, iDepthInTree);
	}
	else // is a leaf
	{
		TreeNodeForRendering tNewLeafNodeForRendering;
		tNewLeafNodeForRendering.m_iDepthInTree = iDepthInTree;
		tNewLeafNodeForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeLeafDataForRendering.push_back(tNewLeafNodeForRendering);
	}
}

void Visualization::TraverseTreeForDataForBottomUpRendering_BoundingSphere(CollisionDetection::BVHTreeNode* pNode, BVHRenderingDataTuple & rBVHRenderDataTuple, int16_t iDepthInTree)
{
	/*
		This is going to be ugly.
		We are facing a problem here: For Bottom up construction of a BVH, the order in which AABBs are constructed is not the same as the tree is later being traversed.
		That means, there is no information in the tree, in which order its nodes were constructed.
		Also, i want to keep this information outside of the tree, since it is really only relevant for rendering/visualizing.
		So, in an effort to keep the visualization only data out of the tree, we have to now do something very ugly.
		Order of construction can only be determined during construction. Tree depth of a given node and its associated AABB can only be determined during traversal.
	*/

	assert(pNode);

	if (pNode->IsANode()) // node -> is a leaf
	{
		// if it is a node, there was a partitioning step, which means there have to be two children
		assert(pNode->m_pLeft);
		assert(pNode->m_pRight);

		// the ugly part. finding the appropriate rendering object to assign it its true tree depth.
		for (TreeNodeForRendering& rCurrentRenderObject : rBVHRenderDataTuple.m_vecTreeNodeDataForRendering)
		{
			if (pNode == rCurrentRenderObject.m_pNodeToBeRendered)
			{
				rCurrentRenderObject.m_iDepthInTree = iDepthInTree;
				break;
			}
		}

		rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes = std::max(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes, iDepthInTree);
		iDepthInTree++; // we are now one level deeper

		// traverse left ...
		TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pLeft, rBVHRenderDataTuple, iDepthInTree);
		// ... then right
		TraverseTreeForDataForBottomUpRendering_AABB(pNode->m_pRight, rBVHRenderDataTuple, iDepthInTree);
	}
	else // is a leaf
	{
		TreeNodeForRendering tNewLeafNodeForRendering;
		tNewLeafNodeForRendering.m_iDepthInTree = iDepthInTree;
		tNewLeafNodeForRendering.m_pNodeToBeRendered = pNode;
		rBVHRenderDataTuple.m_vecTreeLeafDataForRendering.push_back(tNewLeafNodeForRendering);
	}
}

Visualization::TreeNodeForRendering*  Visualization::FindRenderDataOfNode(const CollisionDetection::BVHTreeNode* pNode, std::vector<TreeNodeForRendering>& rvecRenderData) const
{
	TreeNodeForRendering* pResult = nullptr;

	for (TreeNodeForRendering& rCurrentRenderData : rvecRenderData)
	{
		if (rCurrentRenderData.m_pNodeToBeRendered == pNode)
		{
			pResult = &rCurrentRenderData;
			break;
		}
	}

	assert(pResult); // the node you are looking for in the given render data vector does not exist.
	return pResult;
}

void Visualization::LoadTextures()
{
	m_rMainWindow.SetAsCurrentRenderContext();
	//textures 3d
	m_uiObjectDiffuseTexture = Renderer::LoadTextureFromFile("resources/textures/cobblestone_floor_13_diff_1k.jpg");
	m_uiGridMaskTexture = Renderer::LoadTextureFromFile("resources/textures/grid_mask_transparent.png");
	m_uiCrosshairTexture = Renderer::LoadTextureFromFile("resources/textures/crosshair.png");

	m_p2DGraphWindow->SetAsCurrentRenderContext();
	// textures 2D
	m_ui2DCircleTexture = Renderer::LoadTextureFromFile("resources/textures/2DFullcircle.png");
	m_ui2DOBJTexture = Renderer::LoadTextureFromFile("resources/textures/2DOBJ.png");
}

void Visualization::LoadShaders()
{
	m_rMainWindow.SetAsCurrentRenderContext();

	// A simple color shader
	Shader tColorShader("resources/shaders/Color.vs", "resources/shaders/Color.frag");
	m_tColorShader = tColorShader;
	assert(m_tColorShader.IsInitialized());

	// A flat texture shader
	Shader tTextureShader("resources/shaders/FlatTexture.vs", "resources/shaders/FlatTexture.frag");
	m_tFlatTextureShader = tTextureShader;
	assert(m_tFlatTextureShader.IsInitialized());

	// A masked color shader
	Shader tMaskedColorShader("resources/shaders/MaskedColor.vs", "resources/shaders/MaskedColor.frag");
	m_tMaskedColorShader = tMaskedColorShader;
	assert(m_tMaskedColorShader.IsInitialized());

	// A crosshair (hud component) shader
	Shader tHUDComponentColorShader("resources/shaders/MaskedColor2D.vs", "resources/shaders/MaskedColor2D.frag");
	m_tHUDComponentColorShader = tHUDComponentColorShader;
	assert(m_tHUDComponentColorShader.IsInitialized());

	//////////////////////////////////////////////////////////////
	// below is the 2d window
	m_p2DGraphWindow->SetAsCurrentRenderContext();

	Shader tMaskedColorShader2D("resources/shaders/MaskedColor2D.vs", "resources/shaders/MaskedColor2D.frag");
	m_tMaskedColorShader2D = tMaskedColorShader2D;
	assert(m_tMaskedColorShader2D.IsInitialized());

	Shader tColoredLineShader2D("resources/shaders/Colored2DLine.vs", "resources/shaders/Colored2DLine.frag");
	m_tColoredLineShader2D = tColoredLineShader2D;
	assert(m_tColoredLineShader2D.IsInitialized());
}

void Visualization::LoadPrimitivesToGPU()
{
	m_rMainWindow.SetAsCurrentRenderContext();

	// textured cube
	{
		GLuint &rTexturedCubeVBO = m_uiTexturedCubeVBO, &rTexturedCubeVAO = m_uiTexturedCubeVAO, &rTexturedCubeEBO = m_uiTexturedCubeEBO;
		glGenVertexArrays(1, &rTexturedCubeVAO);
		glGenBuffers(1, &rTexturedCubeVBO);
		glGenBuffers(1, &rTexturedCubeEBO);

		glBindVertexArray(rTexturedCubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::VertexData), Primitives::Cube::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::IndexData), Primitives::Cube::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// colored cube
	{
		GLuint &rColoredCubeVBO = m_uiColoredCubeVBO, &rColoredCubeVAO = m_uiColoredCubeVAO, &rColoredCubeEBO = m_uiColoredCubeEBO;
		glGenVertexArrays(1, &rColoredCubeVAO);
		glGenBuffers(1, &rColoredCubeVBO);
		glGenBuffers(1, &rColoredCubeEBO);

		glBindVertexArray(rColoredCubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rColoredCubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Cube::SimpleVertexData), Primitives::Cube::SimpleVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredCubeEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Cube::SimpleIndexData), Primitives::Cube::SimpleIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	// textured plane
	{
		GLuint &rTexturedPlaneVBO = m_uiTexturedPlaneVBO, &rTexturedPlaneVAO = m_uiTexturedPlaneVAO, &rTexturedPlaneEBO = m_uiTexturedPlaneEBO;
		glGenVertexArrays(1, &rTexturedPlaneVAO);
		glGenBuffers(1, &rTexturedPlaneVBO);
		glGenBuffers(1, &rTexturedPlaneEBO);

		glBindVertexArray(rTexturedPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::VertexData), Primitives::Plane::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	// colored plane
	{
		GLuint &rColoredPlaneVBO = m_uiColoredPlaneVBO, &rColoredPlaneVAO = m_uiColoredPlaneVAO, &rColoredPlaneEBO = m_uiColoredPlaneEBO;
		glGenVertexArrays(1, &rColoredPlaneVAO);
		glGenBuffers(1, &rColoredPlaneVBO);
		glGenBuffers(1, &rColoredPlaneEBO);

		glBindVertexArray(rColoredPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rColoredPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Plane::SimpleVertexData), Primitives::Plane::SimpleVertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rColoredPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::SimpleIndexData), Primitives::Plane::SimpleIndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	// a textured sphere
	{
		// generate vertex data first.
		const int iNumberOfIterations = 2;
		Renderer::SphereTrianglesGenerationResult tResult = Renderer::GenerateSphereVertexData(Primitives::Sphere::SphereDefaultRadius, iNumberOfIterations);
		Primitives::Sphere::NumberOfTrianglesInSphere = tResult.m_uiNumberOfTriangles;

		GLuint &rTexturedSphereVBO = m_uiTexturedSphereVBO, &rTexturedSphereVAO = m_uiTexturedSphereVAO;// &rTexturedSphereEBO = m_uiTexturedSphereEBO;
		glGenVertexArrays(1, &rTexturedSphereVAO);
		glGenBuffers(1, &rTexturedSphereVBO);
		//glGenBuffers(1, &rTexturedSphereEBO);

		glBindVertexArray(rTexturedSphereVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedSphereVBO);
		glBufferData(GL_ARRAY_BUFFER, Primitives::Sphere::NumberOfTrianglesInSphere * sizeof(Renderer::TriangularFace), tResult.m_pTriangleData, GL_STATIC_DRAW);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTexturedSphereEBO);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		//delete[] tResult.m_pTriangleData; // todo: this causes a memory leak. (duh) need a cleaner solution for when to create and delete triangle data.
		Primitives::Sphere::VertexData = &tResult.m_pTriangleData->vertex1.vec3Position.x;//reinterpret_cast<GLfloat*>(tResult.m_pTriangleData);
	}

	// plane for uniform grid display
	{
		GLuint &rGridPlaneVBO = m_uiGridPlaneVBO, &rGridPlaneVAO = m_uiGridPlaneVAO, &rGridPlaneEBO = m_uiGridPlaneEBO;
		glGenVertexArrays(1, &rGridPlaneVAO);
		glGenBuffers(1, &rGridPlaneVBO);
		glGenBuffers(1, &rGridPlaneEBO);

		glBindVertexArray(rGridPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rGridPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Specials::GridPlane::VertexData), Primitives::Specials::GridPlane::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rGridPlaneEBO); // index data is equal to that of a normal plane
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	m_p2DGraphWindow->SetAsCurrentRenderContext();

	{
		// 2D planes for "sprite" rendering
		GLuint &rTextured2DPlaneVBO = m_uiTextured2DPlaneVBO, &rTextured2DPlaneVAO = m_uiTextured2DPlaneVAO, &rTextured2DPlaneEBO = m_uiTextured2DPlaneEBO;
		glGenVertexArrays(1, &rTextured2DPlaneVAO);
		glGenBuffers(1, &rTextured2DPlaneVBO);
		glGenBuffers(1, &rTextured2DPlaneEBO);

		glBindVertexArray(rTextured2DPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTextured2DPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::TwoDimensional::UniformPlane::VertexData), Primitives::TwoDimensional::UniformPlane::VertexData, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rTextured2DPlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Primitives::Plane::IndexData), Primitives::Plane::IndexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// normals attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	{
		// Lines
		GLuint &r2DLineVBO = m_ui2DLineVBO, &r2DLineVAO = m_ui2DLineVAO;
		glGenVertexArrays(1, &r2DLineVAO);
		glGenBuffers(1, &r2DLineVBO);

		glBindVertexArray(r2DLineVAO);

		glBindBuffer(GL_ARRAY_BUFFER, r2DLineVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::Line::ColoredLineVertexData), Primitives::Line::ColoredLineVertexData, GL_STATIC_DRAW);

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
}

void Visualization::InitUniformBuffers()
{
	m_rMainWindow.SetAsCurrentRenderContext();

	assert(m_tColorShader.IsInitialized() && m_tFlatTextureShader.IsInitialized()); // need constructed shaders to link

	GLuint& rCameraProjectionUBO = m_uiCameraProjectionUBO;
	glGenBuffers(1, &rCameraProjectionUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, rCameraProjectionUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);	// dynamic draw because the camera matrix will change every frame
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	// defining the range of the buffer, which is 2 mat4s
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, rCameraProjectionUBO, 0, 2 * sizeof(glm::mat4));
}

void Visualization::SetInitialRenderStates()
{
	m_rMainWindow.SetAsCurrentRenderContext();

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glEnable(GL_MULTISAMPLE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(2.0f);
	glEnable(GL_LINE_SMOOTH);
}

Visualization::BVHRenderingDataTuple Visualization::ConstructTopDownAABBBVHandRenderDataForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BVHRenderingDataTuple tResult;

	// the construction
	tResult.m_tBVH.m_pRootNode = new CollisionDetection::BVHTreeNode;
	RecursiveTopDownTree_AABB(&(tResult.m_tBVH.m_pRootNode), rScene.m_vecObjects.data(), rScene.m_vecObjects.size());

	// first traversal to gather data for rendering. In theory, it is possible to traverse the tree every frame for BV rendering.
	// But that is terrible, so data is fetched into a linear vector
	tResult.m_vecTreeNodeDataForRendering.reserve(100);
	TraverseTreeForDataForTopDownRendering_AABB(tResult.m_tBVH.m_pRootNode,tResult, 0);

	// now for the rendering data of the 2d window
	ConstructBVHTreeGraphRenderData(tResult);

	return tResult;
}

Visualization::BVHRenderingDataTuple Visualization::ConstructBottomUpAABBBVHandRenderDataForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BVHRenderingDataTuple tResult;
	tResult.m_vecTreeNodeDataForRendering.reserve(100);

	// the construction INCLUDING HALF THE PREPARATION OF AABB RENDERING DATA
	tResult.m_tBVH.m_pRootNode = BottomUpTree_AABB(rScene.m_vecObjects.data(), rScene.m_vecObjects.size(), tResult);
	// the other half of the rendering data
	TraverseTreeForDataForBottomUpRendering_AABB(tResult.m_tBVH.m_pRootNode, tResult, 0);

	// now for the rendering data of the 2d window
	ConstructBVHTreeGraphRenderData(tResult);

	return tResult;
}

Visualization::BVHRenderingDataTuple Visualization::ConstructTopDownBoundingSphereBVHandRenderDataForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BVHRenderingDataTuple tResult;

	// the construction
	tResult.m_tBVH.m_pRootNode = new CollisionDetection::BVHTreeNode;
	RecursiveTopDownTree_BoundingSphere(&(tResult.m_tBVH.m_pRootNode), rScene.m_vecObjects.data(), rScene.m_vecObjects.size());

	// first traversal to gather data for rendering. In theory, it is possible to traverse the tree every frame for BV rendering.
	// But that is terrible, so data is fetched into a linear vector
	tResult.m_vecTreeNodeDataForRendering.reserve(100);

	TraverseTreeForDataForTopDownRendering_BoundingSphere(tResult.m_tBVH.m_pRootNode, tResult, 0);

	// now for the rendering data of the 2d window
	ConstructBVHTreeGraphRenderData(tResult);

	return tResult;
}

Visualization::BVHRenderingDataTuple Visualization::ConstructBottomUpBoundingSphereBVHandRenderDataForScene(Visualization & rScene)
{
	assert(rScene.m_vecObjects.size() > 0);

	BVHRenderingDataTuple tResult;
	tResult.m_vecTreeNodeDataForRendering.reserve(100);

	// the construction INCLUDING HALF THE PREPARATION OF BOUNDING SPHERE RENDERING DATA
	tResult.m_tBVH.m_pRootNode = BottomUpTree_BoundingSphere(rScene.m_vecObjects.data(), rScene.m_vecObjects.size(), tResult);
	// the other half of the rendering data
	TraverseTreeForDataForBottomUpRendering_BoundingSphere(tResult.m_tBVH.m_pRootNode, tResult, 0);

	// now for the rendering data of the 2d window
	ConstructBVHTreeGraphRenderData(tResult);

	return tResult;
}

void Visualization::ConstructBVHTreeGraphRenderData(BVHRenderingDataTuple& rBVHRenderDataTuple)
{
	// calculate the scaling of of every circle which will represent a node of the tree
	const float fUsableWidth = static_cast<float>(m_p2DGraphWindow->m_iWindowWidth);// *0.9f; // saving 10% space for padding
	// the graph of the binary tree will always be more restricted by its width rather than height, so only that has to be considered
	// also, calculation is based on the assumption of a full tree (all leaves present)

	const float fTreeDepthIncludingLeaves = static_cast<float>(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes + 1);
	const float fTreeWidthIncludingLeaves = std::powf(2.0f, fTreeDepthIncludingLeaves);

	// now need to divide the available space (width) among the number of leaves, resulting in the max size of all nodes
	m_f2DGraphNodeSize = fUsableWidth / fTreeWidthIncludingLeaves;

	// need to calculate the screen space reduction of the tree with each level
	const float fUsableHeight = static_cast<float>(m_p2DGraphWindow->m_iWindowHeight);// *0.9f; // saving 10% space for padding
	m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel = (fUsableHeight - m_f2DGraphNodeSize) / static_cast<float>(rBVHRenderDataTuple.m_tBVH.m_iTDeepestDepthOfNodes + 1);

	ScreenSpaceForGraphRendering tTotalScreenSpace;
	tTotalScreenSpace.m_fWidthStart = 0.0f;
	tTotalScreenSpace.m_fWidthEnd = static_cast<float>(m_p2DGraphWindow->m_iWindowWidth);
	tTotalScreenSpace.m_fHeightStart = 0.0f;
	tTotalScreenSpace.m_fHeightEnd = static_cast<float>(m_p2DGraphWindow->m_iWindowHeight);

	// pre-calculations done, starting with recursion

	RecursiveConstructTreeGraphRenderData(rBVHRenderDataTuple.m_tBVH.m_pRootNode, rBVHRenderDataTuple, tTotalScreenSpace, glm::vec2(-1.0f, -1.0f));
}

void Visualization::RecursiveConstructTreeGraphRenderData(const CollisionDetection::BVHTreeNode * pCurrentNode, BVHRenderingDataTuple& rBVHRenderDataTuple, ScreenSpaceForGraphRendering tScreenSpaceForThisNode, glm::vec2 vec2PreviousDrawPosition)
{
	assert(pCurrentNode);

	glm::vec2 vec2CurrentNodeDrawPosition;
	vec2CurrentNodeDrawPosition.x = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f; // horizontally, in the middle of the given screen space
	vec2CurrentNodeDrawPosition.y = tScreenSpaceForThisNode.m_fHeightEnd - (m_f2DGraphNodeSize * 0.5f);	// vertically, at the top edge of the given screen space

	// drawing the node/leaf
	if (pCurrentNode->IsANode())
	{
		TreeNodeForRendering* pRenderDataOfCurrentNode = FindRenderDataOfNode(pCurrentNode, rBVHRenderDataTuple.m_vecTreeNodeDataForRendering);
		pRenderDataOfCurrentNode->m_vec2_2DNodeDrawPosition = vec2CurrentNodeDrawPosition;
		if (vec2PreviousDrawPosition.x > 0.0f) // little "trick" to distinguish between root and every other node/leaf
		{
			pRenderDataOfCurrentNode->m_vec2_2DLineToParentOrigin = vec2CurrentNodeDrawPosition;
			pRenderDataOfCurrentNode->m_vec2_2DLineToParentTarget = vec2PreviousDrawPosition;
		}
		
		if (pCurrentNode->m_pLeft) {
			ScreenSpaceForGraphRendering tScreenSpaceForLeftChild;
			tScreenSpaceForLeftChild.m_fWidthStart = tScreenSpaceForThisNode.m_fWidthStart;
			tScreenSpaceForLeftChild.m_fWidthEnd = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f;
			tScreenSpaceForLeftChild.m_fHeightStart = tScreenSpaceForThisNode.m_fHeightStart;
			tScreenSpaceForLeftChild.m_fHeightEnd = tScreenSpaceForThisNode.m_fHeightEnd - m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;

			RecursiveConstructTreeGraphRenderData(pCurrentNode->m_pLeft, rBVHRenderDataTuple, tScreenSpaceForLeftChild, vec2CurrentNodeDrawPosition);
		}

		if (pCurrentNode->m_pRight) {
			ScreenSpaceForGraphRendering tScreenSpaceForRightChild;
			tScreenSpaceForRightChild.m_fWidthStart = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f;
			tScreenSpaceForRightChild.m_fWidthEnd = tScreenSpaceForThisNode.m_fWidthEnd;
			tScreenSpaceForRightChild.m_fHeightStart = tScreenSpaceForThisNode.m_fHeightStart;
			tScreenSpaceForRightChild.m_fHeightEnd = tScreenSpaceForThisNode.m_fHeightEnd - m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;

			RecursiveConstructTreeGraphRenderData(pCurrentNode->m_pRight, rBVHRenderDataTuple, tScreenSpaceForRightChild, vec2CurrentNodeDrawPosition);
		}
	}
	else // is a leaf
	{
		TreeNodeForRendering* pRenderDataOfCurrentLeaf = FindRenderDataOfNode(pCurrentNode, rBVHRenderDataTuple.m_vecTreeLeafDataForRendering);
		pRenderDataOfCurrentLeaf->m_vec2_2DNodeDrawPosition = vec2CurrentNodeDrawPosition;
		pRenderDataOfCurrentLeaf->m_vec2_2DLineToParentOrigin = vec2CurrentNodeDrawPosition;
		pRenderDataOfCurrentLeaf->m_vec2_2DLineToParentTarget = vec2PreviousDrawPosition;
	}	
}

void Visualization::DrawNodeAtPosition(glm::vec2 vec2ScreenSpacePosition, const glm::vec4& rvec4DrawColor) const
{
	glAssert();
	const Shader& rCurrentShader = m_tMaskedColorShader2D;
	rCurrentShader.use();
	glAssert();
	rCurrentShader.setInt("transparencyMask", 0);
	glAssert();
	glBindTexture(GL_TEXTURE_2D, m_ui2DCircleTexture);

	glBindVertexArray(m_uiTextured2DPlaneVAO);
	glAssert();

	// world matrix
	glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
	glm::vec3 vec3CircleTranslationVector(vec2ScreenSpacePosition.x, vec2ScreenSpacePosition.y, 0.0f); // in the middle of the window, within the near plane of the view frustum
	mat4World = glm::translate(mat4World, vec3CircleTranslationVector);
	mat4World = glm::scale(mat4World, glm::vec3(m_f2DGraphNodeSize, m_f2DGraphNodeSize, 1.0f));
	rCurrentShader.setMat4("world", mat4World);

	// projection matrix
	rCurrentShader.setMat4("orthoProjection", m_mat4OrthographicProjection2DWindow);

	// setting the color red
	rCurrentShader.setVec4("color", rvec4DrawColor);

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
}

void Visualization::DrawLineFromTo(glm::vec2 vec2From, glm::vec2 vec2To) const
{
	glAssert();
	const Shader& rCurrentShader = m_tColoredLineShader2D;
	rCurrentShader.use();

	
	const glm::vec2 vec2LineDirection = vec2To - vec2From;
	const float fLineLength = glm::length(vec2LineDirection);
	float fRotationAngle = std::acos(glm::dot(glm::normalize(vec2LineDirection), glm::vec2(0.0f, 1.0f))); // inverse cosine function of the dot product to get the angle between the default line direction and the current one.
	if (vec2To.x > vec2From.x)		// need to rotate in the other direction when end point is "right" to start point
		fRotationAngle *= -1.0f;

	// calc world matrix
	glm::mat4 world = glm::mat4(1.0f); // starting with identity matrix
	// translation: move origin point to given coordinate on screen
	glm::vec3 vec3Translation(vec2From.x, vec2From.y, 0.0f);
	world = glm::translate(world, vec3Translation);
	// scale: base line has length of 1, scale it to its intended length
	world = glm::scale(world, glm::vec3(fLineLength, fLineLength, fLineLength));
	// rotation: the upward pointing line is rotated around the Z axis
	world = glm::rotate(world, fRotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));

	rCurrentShader.setMat4("world", world);

	// projection matrix
	rCurrentShader.setMat4("orthoProjection", m_mat4OrthographicProjection2DWindow);

	// setting the color blue
	rCurrentShader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	glAssert();

	// render the line
	glBindVertexArray(m_ui2DLineVAO);
	glDrawArrays(GL_LINES, 0, 2);
}

void Visualization::Draw2DObjectAtPosition(glm::vec2 vec2ScreenSpacePosition, const glm::vec4& rvec4DrawColor) const
{
	glAssert();
	const Shader& rCurrentShader = m_tMaskedColorShader2D;
	rCurrentShader.use();
	glAssert();
	rCurrentShader.setInt("transparencyMask", 0);
	glAssert();
	glBindTexture(GL_TEXTURE_2D, m_ui2DOBJTexture);

	glBindVertexArray(m_uiTextured2DPlaneVAO);
	glAssert();

	// world matrix
	glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
	glm::vec3 vec3CircleTranslationVector(vec2ScreenSpacePosition.x, vec2ScreenSpacePosition.y, 0.0f); // in the middle of the window, within the near plane of the view frustum
	mat4World = glm::translate(mat4World, vec3CircleTranslationVector);
	mat4World = glm::scale(mat4World, glm::vec3(m_f2DGraphNodeSize, m_f2DGraphNodeSize, 1.0f));
	rCurrentShader.setMat4("world", mat4World);

	// projection matrix
	rCurrentShader.setMat4("orthoProjection", m_mat4OrthographicProjection2DWindow);

	// setting the color
	rCurrentShader.setVec4("color", rvec4DrawColor);

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
}

void Visualization::ShowObjectPropertiesWindow(bool bShowIt)
{
	m_bShowObjectPropertiesWindow = bShowIt;

	// tell the backup that it needs an update
	tObjectPropertiesBackup.m_bValid = false;
}

void Visualization::ToggleHelpWindow()
{
	m_bShowHelpWindow = !m_bShowHelpWindow;
}

void Visualization::RenderSimControlPanel()
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
	if (m_bGUICaptureMouse == false)
		window_flags |= ImGuiWindowFlags_NoMouseInputs;

	std::string sControlPanelName("CONTROLS for: ");

	switch (m_eConstructionStrategy)
	{
	case eBVHConstructionStrategy::TOPDOWN:
		sControlPanelName.append("TOP DOWN ");
		break;
	case eBVHConstructionStrategy::BOTTOMUP:
		sControlPanelName.append("BOTTOM UP ");
		break;
	default:
		assert(!"disaster");
		break;
	}

	switch (m_eBVHBoundingVolume)
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
		ResetSimulation();
	ImGui::SameLine();

	if (m_ePresentationMode == Visualization::ePresentationMode::DISCRETE)
	{
		if (ImGui::Button("PLAY"))
			PlaySimulation();
	}
	else
	{
		if (ImGui::Button("PAUSE"))
			PauseSimulation();
	}
	ImGui::SameLine();
	if (ImGui::Button("STEP"))
		MoveToNextSimulationStep();
	ImGui::SameLine();
	std::string sInverButtonLabel = (m_iSimulationDirectionSign == 1) ? "REVERSE PLAYBACK" : "FORWARD PLAYBACK";
	if (ImGui::Button(sInverButtonLabel.c_str()))
		InvertSimulationProgression();


	if (ImGui::Button("SLOWER"))
		DecreaseSimulationSpeed();
	ImGui::SameLine();
	ImGui::Text("Speed: %.2f", GetCurrentSimulationSpeed());
	ImGui::SameLine();
	if (ImGui::Button("FASTER"))
		IncreaseSimulationSpeed();

	ImGui::Separator();
	if (ImGui::Button("SIMULATION OPTIONS"))
		m_bShowSimulationOptions = !m_bShowSimulationOptions;
	ImGui::SameLine();
	if (ImGui::Button("HELP"))
		ToggleHelpWindow();

	ImGui::End();
}

void Visualization::RenderSimOptions()
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
	if (m_bGUICaptureMouse == false)
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
				LoadDefaultScene();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine(); GUI::HelpMarker("The scene which is loaded when you first start the Bounding Volume Hierarchy Visualization");

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
			ImGui::Text("Are you sure?"); ImGui::SameLine(); GUI::HelpMarker("Deleting all objects will reconstruct the Bounding Volume Hierarchy and resest the simulation.");
			if (ImGui::Button("YES", ImVec2(120, 0)))
			{
				ClearCurrentScene();
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

	ImGui::Text("BVH Bounding Volume"); ImGui::SameLine(); GUI::HelpMarker("The Hierarchy's nodes' Bounding Volume");
	// The combo box to choose the hierarchy's node bounding volume
	const char* pBVHBoundingVolumeItems[] = { "AABB", "Bounding Sphere" };
	int iCurrentBVHBoundingVolumeItemIndex = static_cast<int>(m_eBVHBoundingVolume);
	const char* sBVHBoundingVolumeComboLabel = pBVHBoundingVolumeItems[iCurrentBVHBoundingVolumeItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("##BVH Bounding Volume", sBVHBoundingVolumeComboLabel))
	{
		for (int iCurrentItem = 0; iCurrentItem < IM_ARRAYSIZE(pBVHBoundingVolumeItems); iCurrentItem++)
		{
			const bool bIsSelected = (iCurrentBVHBoundingVolumeItemIndex == iCurrentItem);
			if (ImGui::Selectable(pBVHBoundingVolumeItems[iCurrentItem], bIsSelected))
			{
				assert(iCurrentItem <= static_cast<int>(Visualization::eBVHBoundingVolume::NUM_BVHBOUNDINGVOLUMES));
				SetNewBVHBoundingVolume(static_cast<Visualization::eBVHBoundingVolume>(iCurrentItem));
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
	int iCurrentConstructionStrategyItemIndex = static_cast<int>(m_eConstructionStrategy);
	const char* sConstructionStrategyComboLabel = pBVHConstructionStrategyItems[iCurrentConstructionStrategyItemIndex];  // Label to preview before opening the combo (technically it could be anything)
	if (ImGui::BeginCombo("##BVH Construction Strategy", sConstructionStrategyComboLabel))
	{
		for (int iCurrentItem = 0; iCurrentItem < IM_ARRAYSIZE(pBVHConstructionStrategyItems); iCurrentItem++)
		{
			const bool bIsSelected = (iCurrentConstructionStrategyItemIndex == iCurrentItem);
			if (ImGui::Selectable(pBVHConstructionStrategyItems[iCurrentItem], bIsSelected))
			{
				assert(iCurrentItem <= static_cast<int>(Visualization::eBVHConstructionStrategy::NUM_BVHCONSTRUCTIONSTRATEGIES));
				SetNewBVHConstructionStrategy(static_cast<Visualization::eBVHConstructionStrategy>(iCurrentItem));
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
		ImGui::ColorEdit3("Node Color##TOPDOWN", (float*)&m_vec4TopDownNodeRenderColor, iColorPickerFlags); ImGui::SameLine();
		ImGui::Checkbox("Gradient##TOPDOWN", &m_bNodeDepthColorGrading);
		if (m_bNodeDepthColorGrading)
			ImGui::ColorEdit3("Node Gradient Color##TOPDOWN", (float*)&m_vec4TopDownNodeRenderColor_Gradient, iColorPickerFlags);
	}

	// BOTTOM UP OPTIONS
	if (iCurrentConstructionStrategyItemIndex == 1)
	{
		ImGui::Text("BOTTOM UP OPTIONS AND PARAMETERS");
		ImGui::ColorEdit3("Node Color##BOTTOMUP", (float*)&m_vec4BottomUpNodeRenderColor, iColorPickerFlags); ImGui::SameLine();
		ImGui::Checkbox("Gradient##BOTTOMUP", &m_bNodeDepthColorGrading);
		if (m_bNodeDepthColorGrading)
			ImGui::ColorEdit3("Node Gradient Color##BOTTOMUP", (float*)&m_vec4BottomUpNodeRenderColor_Gradient, iColorPickerFlags);
	}



	//if (ImGui::Button("Rebuild BVHs"))
	//{
	//	assert(!"new software architecture, reconsider");
	//	CollisionDetection::UpdateBoundingVolumesForScene(rEngine.m_pVisualization);
	//	rEngine.m_tTopDownBVH_AABB = CollisionDetection::ConstructTopDownAABBBVHandRenderDataForScene(rEngine.m_pVisualization);
	//	rEngine.m_tBottomUpBVH_AABB = CollisionDetection::ConstructBottomUpAABBBVHandRenderDataForScene(rEngine.m_pVisualization);
	//	rEngine.m_pVisualization->m_bBVHTreesValid = true;
	//}

	ImGui::Separator();

	ImGui::Text("Object AABBs");
	ImGui::ColorEdit3("Color##AABB", (float*)&m_vec4AABBColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##Draw AABBs of Objects", &m_bRenderObjectAABBs);
	ImGui::Text("Object Bounding Spheres");
	ImGui::ColorEdit3("Color##BoundingSphere", (float*)&m_vec4BoundingSphereColor, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##Draw Bounding Spheres of Objects", &m_bRenderObjectBoundingSpheres);

	ImGui::Separator();

	// X axis grid
	ImGui::Text("X Axis Grid"); ImGui::SameLine(); GUI::HelpMarker("A rasterized grid facing the X axis, spanning the YZ plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##X", (float*)&m_vec4GridColorX, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxXGrid", &m_bRenderGridXPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Xdown", ImGuiDir_Down)) { m_vec3GridPositionsOnAxes.x -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Xup", ImGuiDir_Up)) { m_vec3GridPositionsOnAxes.x += 10.0f; }
	ImGui::PopButtonRepeat();
	// Y axis grid
	ImGui::Text("Y Axis Grid"); ImGui::SameLine(); GUI::HelpMarker("A rasterized grid facing the Y axis, spanning the XZ plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##Y", (float*)&m_vec4GridColorY, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxYGrid", &m_bRenderGridYPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Ydown", ImGuiDir_Down)) { m_vec3GridPositionsOnAxes.y -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Yup", ImGuiDir_Up)) { m_vec3GridPositionsOnAxes.y += 10.0f; }
	ImGui::PopButtonRepeat();
	// Z axis grid
	ImGui::Text("Z Axis Grid"); ImGui::SameLine(); GUI::HelpMarker("A rasterized grid facing the Z axis, spanning the XY plane. Grid size = 100cm");
	ImGui::ColorEdit3("Color##Z", (float*)&m_vec4GridColorZ, iColorPickerFlags); ImGui::SameLine();
	ImGui::Checkbox("Visibility##checkBoxZGrid", &m_bRenderGridZPlane); ImGui::SameLine();
	ImGui::PushButtonRepeat(true);
	if (ImGui::ArrowButton("##Zdown", ImGuiDir_Down)) { m_vec3GridPositionsOnAxes.z -= 10.0f; }
	ImGui::SameLine();
	if (ImGui::ArrowButton("##Zup", ImGuiDir_Up)) { m_vec3GridPositionsOnAxes.z += 10.0f; }
	ImGui::PopButtonRepeat();

	ImGui::Separator();
	ImGui::Text("Crosshair");
	ImGui::ColorEdit3("Color##XHAIR", (float*)&m_vec4CrossHairColor, iColorPickerFlags);
	ImGui::SliderFloat("Size", &m_fCrossHairScaling, 0.75f, 2.0f, "%.2f", ImGuiSliderFlags_NoInput);

	ImGui::End();
}

void Visualization::RenderObjectPropertiesWindow()
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(350, 250);

	ImGui::SetNextWindowPos(m_vec2ObjectPropertiesWindowPosition, ImGuiCond_Always);
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bGUICaptureMouse == false)
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
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z]");
	if (ImGui::InputFloat3("Scale", &(m_pCurrentlyFocusedObject->m_tTransform.m_vec3Scale.x), "%.2f", flags))
		m_bObjectPropertiesPendingChanges = true;
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z]");
	if (ImGui::InputFloat4("Rotation", &(m_pCurrentlyFocusedObject->m_tTransform.m_tRotation.m_vec3Axis.x), "%.2f", flags))
		m_bObjectPropertiesPendingChanges = true;
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z][angle]");
	if (ImGui::Button("DELETE OBJECT"))
	{
		ImGui::OpenPopup("CONFIRM OBJECT DELETION");
	}
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("CONFIRM OBJECT DELETION", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure?"); ImGui::SameLine(); GUI::HelpMarker("Deleting this object will reconstruct the Bounding Volume Hierarchy and resest the simulation. It cannot be undone.");
			if (ImGui::Button("YES", ImVec2(120, 0)))
			{
				m_bShowObjectPropertiesWindow = false; // closes the window

				DeleteGivenObject(m_pCurrentlyFocusedObject);
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
			UpdateAfterObjectPropertiesChange();
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

void Visualization::RenderObjectCreationWindow()
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(350, 250);

	ImGui::SetNextWindowPos(main_viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bGUICaptureMouse == false)
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
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z]");
	ImGui::InputFloat3("Scale", &(pObjectCreationData->m_tTransform.m_vec3Scale.x), "%.2f", flags);
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z]");
	ImGui::InputFloat4("Rotation", &(pObjectCreationData->m_tTransform.m_tRotation.m_vec3Axis.x), "%.2f", flags);
	ImGui::SameLine(); GUI::HelpMarker("[X][Y][Z][angle]");

	ImGui::TextWrapped("Creating a new object [OK] will invalidate the BVH. It will automatically be reconstructed and the visualization reset.");

	if (ImGui::Button("OK"))
	{
		AddNewSceneObject(tObjectCreationViewModel.m_tSceneObject);

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

void Visualization::RenderHelpWindow()
{
	const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

	// configure window
	ImVec2 objectPropertiesWindowSize(450, 450);

	ImGui::SetNextWindowPos(main_viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(objectPropertiesWindowSize, ImGuiCond_Always);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	if (m_bGUICaptureMouse == false)
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

void Visualization::CancelObjectPropertiesChanges()
{
	assert(m_pCurrentlyFocusedObject);
	if (m_bObjectPropertiesPendingChanges)
		m_pCurrentlyFocusedObject->m_tTransform = tObjectPropertiesBackup.m_tBackedupData;

	m_bShowObjectPropertiesWindow = false; // closes this window
	tObjectPropertiesBackup.m_bValid = false; // future backup data will need to be fetched again
	m_bObjectPropertiesPendingChanges = false; // reset this flag
}

bool Visualization::IsMouseCaptured() const
{
	return m_bGUICaptureMouse;
}

void Visualization::SetCaptureMouse(bool bIsCapturedNow)
{
	m_bGUICaptureMouse = bIsCapturedNow;
}

void Visualization::SetFocusedObject(SceneObject * pFocusedObject)
{
	// nullptr for paramenter is valid -> no object in focus
	m_pCurrentlyFocusedObject = pFocusedObject;
}

void Visualization::SetObjectPropertiesWindowPosition(float fXPosition, float fYPosition)
{
	m_vec2ObjectPropertiesWindowPosition = ImVec2(fXPosition, fYPosition);
}

void Visualization::CrosshairClick()
{
	glm::vec3 vec3RayDirection = vec3RayDirection = m_tCamera.GetFrontVector();

	// construct ray
	CollisionDetection::Ray tRay(m_tCamera.GetCurrentPosition(), vec3RayDirection);

	// check for intersections with ray
	//CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::CastRayIntoBVH(pEngine->m_pVisualization->m_tTopDownBVH_AABB, tRay);
	CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::BruteForceRayIntoObjects(m_vecObjects, tRay);

	SceneObject* pPreviouslyFocusedObject = m_pCurrentlyFocusedObject;
	if (pPreviouslyFocusedObject)	// if there was an object in focus before this click, cancel any pending changes made to it
		CancelObjectPropertiesChanges();

	// execute orders in accordance with the result
	if (tResult.IntersectionWithObjectOccured())
	{
		m_pCurrentlyFocusedObject = tResult.m_pFirstIntersectedSceneObject;
		m_vec2ObjectPropertiesWindowPosition = ImVec2(static_cast<float>(m_rMainWindow.m_iWindowWidth * 0.5f), static_cast<float>(m_rMainWindow.m_iWindowHeight *0.5f));
		m_bShowObjectPropertiesWindow = true;
	}
	else
	{
		m_pCurrentlyFocusedObject = nullptr;
	}
}

void Visualization::CursorClick()
{
	glm::vec3 vec3RayDirection = Renderer::ConstructRayDirectionFromMousePosition(m_rMainWindow, m_mat4PerspectiveProjection3DWindow, m_mat4Camera);

	// construct ray
	CollisionDetection::Ray tRay(m_tCamera.GetCurrentPosition(), vec3RayDirection);

	// check for intersections with ray
	//CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::CastRayIntoBVH(pEngine->m_pVisualization->m_tTopDownBVH_AABB, tRay);
	CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::BruteForceRayIntoObjects(m_vecObjects, tRay);

	SceneObject* pPreviouslyFocusedObject = m_pCurrentlyFocusedObject;
	if (pPreviouslyFocusedObject)	// if there was an object in focus before this click, cancel any pending changes made to it
		CancelObjectPropertiesChanges();

	// execute orders in accordance with the result
	if (tResult.IntersectionWithObjectOccured())
	{
		m_pCurrentlyFocusedObject = tResult.m_pFirstIntersectedSceneObject;
		m_vec2ObjectPropertiesWindowPosition = ImVec2(m_rMainWindow.GetCurrentMousePosition().m_fXPosition, m_rMainWindow.GetCurrentMousePosition().m_fYPosition);
		m_bShowObjectPropertiesWindow = true;
	}
	else
	{
		m_pCurrentlyFocusedObject = nullptr;
	}
}
