#include "Visualization.h"

#include <assert.h>
#include <algorithm>
#include <limits>
#include <cmath>

#include "glm/glm.hpp"

#include "Engine.h"
#include "GeometricPrimitiveData.h"
#include "Renderer.h"

Visualization::Visualization() :
	m_p2DGraphWindow(nullptr),
	m_iMaximumRenderedTreeDepth(),
	m_iNumberStepsRendered(0),
	m_ePresentationMode(DISCRETE),
	m_eConstructionStrategy(TOPDOWN),
	m_eBVHBoundingVolume(AABB),
	m_fCrossHairScaling(1.0f),
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
	m_bNodeDepthColorGrading(false)
{
	InitPlaybackSpeeds();
	InitRenderColors();
	ResetSimulation();
}

Visualization::~Visualization()
{
	delete m_p2DGraphWindow;

	m_tTopDownBVH_AABB.DeleteTree();
	m_tBottomUpBVH_AABB.DeleteTree();
	m_tTopDownBVH_BoundingSphere.DeleteTree();
	m_tBottomUpBVH_BoundingSphere.DeleteTree();
}

void Visualization::Load()
{
	m_p2DGraphWindow = Engine::MakeWindow(640, 480, "BVH Tree");
	GLFWwindow* pPreviousContext = glfwGetCurrentContext();
	m_p2DGraphWindow->SetAsCurrentContext();

	// create all gl relevant resources for the context of the second window.
	// only container objects (https://www.khronos.org/opengl/wiki/OpenGL_Object#Container_objects) cannot be shared between contexts, and MUST be created for each context
	// However, for now the decision was made that no objects are going to be shared -> create all of them

	// shaders
	Shader tMaskedColorShader2D("resources/shaders/MaskedColor2D.vs", "resources/shaders/MaskedColor2D.frag");
	m_tMaskedColorShader2D = tMaskedColorShader2D;
	assert(m_tMaskedColorShader2D.IsInitialized());

	Shader tColoredLineShader2D("resources/shaders/Colored2DLine.vs", "resources/shaders/Colored2DLine.frag");
	m_tColoredLineShader2D = tColoredLineShader2D;
	assert(m_tColoredLineShader2D.IsInitialized());

	// textures
	m_ui2DCircleTexture = Renderer::LoadTextureFromFile("resources/textures/2Dcircle.png");
	m_ui2DOBJTexture = Renderer::LoadTextureFromFile("resources/textures/2DOBJ.png");

	{
		// objects.. buffer, VAO
		GLuint &rTexturedPlaneVBO = m_uiTexturedPlaneVBO, &rTexturedPlaneVAO = m_uiTexturedPlaneVAO, &rTexturedPlaneEBO = m_uiTexturedPlaneEBO;
		glGenVertexArrays(1, &rTexturedPlaneVAO);
		glGenBuffers(1, &rTexturedPlaneVBO);
		glGenBuffers(1, &rTexturedPlaneEBO);

		glBindVertexArray(rTexturedPlaneVAO);

		glBindBuffer(GL_ARRAY_BUFFER, rTexturedPlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Primitives::TwoDimensional::UniformPlane::VertexData), Primitives::TwoDimensional::UniformPlane::VertexData, GL_STATIC_DRAW);

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
	
	{
		// objects.. buffer, VAO
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

	glfwMakeContextCurrent(pPreviousContext);

	LoadDefaultScene();
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

	//for (int uiCurrentNewObject = 0; uiCurrentNewObject < vecNewObjectsPositions.size(); uiCurrentNewObject++)
	for (int uiCurrentNewObject = 0; uiCurrentNewObject < 4; uiCurrentNewObject++)
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

void Visualization::ReconstructAllTrees()
{
	m_tTopDownBVH_AABB.DeleteTree();
	m_tTopDownBVH_AABB = CollisionDetection::ConstructTopDownAABBBVHForScene(*this);

	m_tBottomUpBVH_AABB.DeleteTree();
	m_tBottomUpBVH_AABB = CollisionDetection::ConstructBottomUpAABBBVHForScene(*this);

	m_tTopDownBVH_BoundingSphere.DeleteTree();
	m_tTopDownBVH_BoundingSphere = CollisionDetection::ConstructTopDownBoundingSphereBVHForScene(*this);

	m_tBottomUpBVH_BoundingSphere.DeleteTree();
	m_tBottomUpBVH_BoundingSphere = CollisionDetection::ConstructBottomUpBoundingSphereBVHForScene(*this);
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
	if (m_ePresentationMode == CONTINUOUS)
	{
		const float fPlaybackSpeedAdjustedDeltaTime = fDeltaTime * m_pPlaybackSpeeds[m_uiCurrentPlayBackSpeedIndex]; // at a playback speed of 0.25, time is 4 times slower for the simulation
		m_fAccumulatedTimeSinceLastUpdateStep += fPlaybackSpeedAdjustedDeltaTime;
		if (m_fAccumulatedTimeSinceLastUpdateStep >= 1.0f)
		{
			MoveToNextSimulationStep();
			m_fAccumulatedTimeSinceLastUpdateStep -= 1.0f;
		}
	}
}

void Visualization::RenderAdditionalWindows(Renderer& rRenderer) const
{
	if (m_p2DGraphWindow) // quick and dirty "fix" to only try to render when the second window actually exists
	{
		GLFWwindow* pPreviousContext = glfwGetCurrentContext();

		glfwMakeContextCurrent(m_p2DGraphWindow->m_pGLFWwindow);

		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/////////////////////////////////////////////////////////
		// CIRCLES
		//{
		//	DrawNodeAtPosition(glm::vec2(static_cast<float>(m_p2DGraphWindow->m_iWindowWidth) * 0.5f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight) * 0.5f));
		//}

		///////////////////////////////////////////////////////////
		//// LINES
		//{
		//	glm::vec2 from(static_cast<float>(m_p2DGraphWindow->m_iWindowWidth) * 0.5f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight) * 0.5f);
		//	glm::vec2 to(from.x + 100.0f, from.y + 100.0f);

		//	DrawLineFromTo(from, to);
		//}
		DrawBVHTreeGraph(m_tTopDownBVH_AABB);


		/////////////////////////////////////////////////////////

		glfwSwapBuffers(m_p2DGraphWindow->m_pGLFWwindow);

		glfwMakeContextCurrent(pPreviousContext);
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
	assert(m_vecTreeAABBsForTopDownRendering.size() <= std::numeric_limits<int>::max());	// make sure that number fits or chaos might ensue.
	m_iNumberStepsRendered = std::min<int>(m_iNumberStepsRendered, static_cast<int>(m_vecTreeAABBsForTopDownRendering.size()));
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

	// deleting the trees too
	m_tTopDownBVH_AABB.DeleteTree();
	m_tBottomUpBVH_AABB.DeleteTree();
	m_tTopDownBVH_BoundingSphere.DeleteTree();
	m_tBottomUpBVH_BoundingSphere.DeleteTree();

	// also deleting the rendering data
	m_vecTreeAABBsForTopDownRendering.clear();
	m_vecTreeAABBsForBottomUpRendering.clear();
	m_vecTreeBoundingSpheresForTopDownRendering.clear();
	m_vecTreeBoundingSpheresForBottomUpRendering.clear();
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
	// grid
	m_vec4GridColorX = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4GridColorY = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); // cyan
	m_vec4GridColorZ = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple

	// crosshair
	m_vec4CrossHairColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // green

	// bounding volumes
	m_vec4AABBDefaultColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // yellow
	m_vec4BoundingSphereDefaultColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // blue

	// node colors
	m_vec4TopDownNodeRenderColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // red
	m_vec4TopDownNodeRenderColor_Gradient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // black
	m_vec4BottomUpNodeRenderColor = glm::vec4(1.0f, 0.0f, 1.0f, 1.0f); // purple
	m_vec4BottomUpNodeRenderColor_Gradient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // black
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

void Visualization::DrawBVHTreeGraph(const CollisionDetection::BoundingVolumeHierarchy& rBVH) const
{
	assert(glfwGetCurrentContext() == m_p2DGraphWindow->m_pGLFWwindow); // you are not in the right context!

	const Shader& rCurrentShader = m_tMaskedColorShader2D;

	// TODO: move this calculation out into after the tree is constructed

	// calculate the scaling of of every circle which will represent a node of the tree
	const float fUsableWidth = static_cast<float>(m_p2DGraphWindow->m_iWindowWidth);// *0.9f; // saving 10% space for padding
	// the graph of the binary tree will always be more restricted by its width rather than height, so only that has to be considered
	// also, calculation is based on the assumption of a full tree (all leaves present)

	const float fTreeDepthIncludingLeaves = static_cast<float>(rBVH.m_iTDeepestDepthOfNodes + 1);
	const float fTreeWidthIncludingLeaves = std::powf(2.0f, fTreeDepthIncludingLeaves);

	// now need to divide the available space (width) among the number of leaves, resulting in the max size of all nodes
	m_f2DGraphNodeSize = fUsableWidth / fTreeWidthIncludingLeaves;

	// need to calculate the screen space reduction of the tree with each level
	const float fUsableHeight = static_cast<float>(m_p2DGraphWindow->m_iWindowHeight);// *0.9f; // saving 10% space for padding
	m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel = (fUsableHeight - m_f2DGraphNodeSize) / static_cast<float>(rBVH.m_iTDeepestDepthOfNodes + 1);

	ScreenSpaceForGraphRendering tTotalScreenSpace;
	tTotalScreenSpace.m_fWidthStart = 0.0f;
	tTotalScreenSpace.m_fWidthEnd = static_cast<float>(m_p2DGraphWindow->m_iWindowWidth);
	tTotalScreenSpace.m_fHeightStart = 0.0f;
	tTotalScreenSpace.m_fHeightEnd = static_cast<float>(m_p2DGraphWindow->m_iWindowHeight);

	// pre-calculations done, starting with recursion

	RecursiveDrawTreeGraph(rBVH.m_pRootNode, tTotalScreenSpace, glm::vec2(-1.0f, -1.0f));

	
	

	/*int16_t iAlreadyRenderedConstructionSteps = 0;
	for (const CollisionDetection::TreeNodeForRendering& rCurrentRenderedBVHBoundingSphere : rvecNodesToRender)
	{
		bool bIsWithinMaximumRenderedTreeDepth = (rCurrentRenderedBVHBoundingSphere.m_iDepthInTree <= m_iMaximumRenderedTreeDepth);
		bool bIsWithinMaximumRenderedConstructionSteps = (iAlreadyRenderedConstructionSteps < m_iNumberStepsRendered);

		bool bShallRender = bIsWithinMaximumRenderedConstructionSteps && bIsWithinMaximumRenderedTreeDepth;
		if (bShallRender)
		{
			
		}

		iAlreadyRenderedConstructionSteps++;
	}*/
}

void Visualization::RecursiveDrawTreeGraph(const CollisionDetection::BVHTreeNode * pCurrentNode, ScreenSpaceForGraphRendering tScreenSpaceForThisNode, glm::vec2 vec2PreviousDrawPosition) const
{
	assert(pCurrentNode);

	glm::vec2 vec2CurrentNodeDrawPosition;
	vec2CurrentNodeDrawPosition.x = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f; // horizontally, in the middle of the given screen space
	vec2CurrentNodeDrawPosition.y = tScreenSpaceForThisNode.m_fHeightEnd - (m_f2DGraphNodeSize * 0.5f);	// vertically, at the top edge of the given screen space

	// drawing the node/leaf
	if (pCurrentNode->IsANode())
	{
		DrawNodeAtPosition(vec2CurrentNodeDrawPosition);

		if (pCurrentNode->m_pLeft) {
			ScreenSpaceForGraphRendering tScreenSpaceForLeftChild;
			tScreenSpaceForLeftChild.m_fWidthStart = tScreenSpaceForThisNode.m_fWidthStart;
			tScreenSpaceForLeftChild.m_fWidthEnd = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f;
			tScreenSpaceForLeftChild.m_fHeightStart = tScreenSpaceForThisNode.m_fHeightStart;
			tScreenSpaceForLeftChild.m_fHeightEnd = tScreenSpaceForThisNode.m_fHeightEnd - m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;

			RecursiveDrawTreeGraph(pCurrentNode->m_pLeft, tScreenSpaceForLeftChild, vec2CurrentNodeDrawPosition);
		}

		if (pCurrentNode->m_pRight) {
			ScreenSpaceForGraphRendering tScreenSpaceForRightChild;
			tScreenSpaceForRightChild.m_fWidthStart = tScreenSpaceForThisNode.m_fWidthStart + (tScreenSpaceForThisNode.m_fWidthEnd - tScreenSpaceForThisNode.m_fWidthStart) * 0.5f;
			tScreenSpaceForRightChild.m_fWidthEnd = tScreenSpaceForThisNode.m_fWidthEnd;
			tScreenSpaceForRightChild.m_fHeightStart = tScreenSpaceForThisNode.m_fHeightStart;
			tScreenSpaceForRightChild.m_fHeightEnd = tScreenSpaceForThisNode.m_fHeightEnd - m_f2DGraphVerticalScreenSpaceReductionPerTreeLevel;

			RecursiveDrawTreeGraph(pCurrentNode->m_pRight, tScreenSpaceForRightChild, vec2CurrentNodeDrawPosition);
		}
	}
	else // is a leaf
	{
		Draw2DObjectAtPosition(vec2CurrentNodeDrawPosition);
	}

	// drawing the line between this node/leaf and its parent
	if (vec2PreviousDrawPosition.x > 0.0f) // little "trick" to distinguish between root and every other node/leaf
		DrawLineFromTo(vec2CurrentNodeDrawPosition, vec2PreviousDrawPosition);
}

void Visualization::DrawNodeAtPosition(glm::vec2 vec2ScreenSpacePosition) const
{
	glAssert();
	const Shader& rCurrentShader = m_tMaskedColorShader2D;
	rCurrentShader.use();
	glAssert();
	rCurrentShader.setInt("transparencyMask", 0);
	glAssert();
	glBindTexture(GL_TEXTURE_2D, m_ui2DCircleTexture);

	glBindVertexArray(m_uiTexturedPlaneVAO);
	glAssert();

	// world matrix
	glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
	glm::vec3 vec3CircleTranslationVector(vec2ScreenSpacePosition.x, vec2ScreenSpacePosition.y, 0.0f); // in the middle of the window, within the near plane of the view frustum
	mat4World = glm::translate(mat4World, vec3CircleTranslationVector);
	mat4World = glm::scale(mat4World, glm::vec3(m_f2DGraphNodeSize, m_f2DGraphNodeSize, m_f2DGraphNodeSize));
	rCurrentShader.setMat4("world", mat4World);

	// projection matrix
	glm::mat4 mat4OrthoProjection2DWindow = glm::ortho(0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowWidth), 0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight), -0.1f, 10.0f);
	rCurrentShader.setMat4("orthoProjection", mat4OrthoProjection2DWindow);

	// setting the color red
	rCurrentShader.setVec4("color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

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
	if (vec2To.x > vec2From.x)		// need to rotate in the other direction.
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
	glm::mat4 mat4OrthoProjection2DWindow = glm::ortho(0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowWidth), 0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight), -0.1f, 10.0f);
	rCurrentShader.setMat4("orthoProjection", mat4OrthoProjection2DWindow);

	// setting the color blue
	rCurrentShader.setVec4("color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

	glAssert();

	// render the line
	glBindVertexArray(m_ui2DLineVAO);
	glDrawArrays(GL_LINES, 0, 2);
}

void Visualization::Draw2DObjectAtPosition(glm::vec2 vec2ScreenSpacePosition) const
{
	glAssert();
	const Shader& rCurrentShader = m_tMaskedColorShader2D;
	rCurrentShader.use();
	glAssert();
	rCurrentShader.setInt("transparencyMask", 0);
	glAssert();
	glBindTexture(GL_TEXTURE_2D, m_ui2DOBJTexture);

	glBindVertexArray(m_uiTexturedPlaneVAO);
	glAssert();

	// world matrix
	glm::mat4 mat4World = glm::mat4(1.0f); // init to identity
	glm::vec3 vec3CircleTranslationVector(vec2ScreenSpacePosition.x, vec2ScreenSpacePosition.y, 0.0f); // in the middle of the window, within the near plane of the view frustum
	mat4World = glm::translate(mat4World, vec3CircleTranslationVector);
	mat4World = glm::scale(mat4World, glm::vec3(m_f2DGraphNodeSize, m_f2DGraphNodeSize, m_f2DGraphNodeSize));
	rCurrentShader.setMat4("world", mat4World);

	// projection matrix
	glm::mat4 mat4OrthoProjection2DWindow = glm::ortho(0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowWidth), 0.0f, static_cast<float>(m_p2DGraphWindow->m_iWindowHeight), -0.1f, 10.0f);
	rCurrentShader.setMat4("orthoProjection", mat4OrthoProjection2DWindow);

	// setting the color
	rCurrentShader.setVec4("color", glm::vec4(0.0f, 0.0f, 0.55f, 1.0f));

	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sizeof(Primitives::Plane::IndexData) / sizeof(GLuint)), GL_UNSIGNED_INT, 0);
}
