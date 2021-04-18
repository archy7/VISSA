#include "Engine.h"

#include "SceneObject.h"

Engine* Engine::sm_pEngine = nullptr;

Engine::Engine() :
	m_pMainWindow(nullptr),
	m_tRenderer(),
	m_tCamera(glm::vec3(0.0f, 0.0f, 0.0f)),
	m_tGUI(),
	m_tVisualization(),
	m_iDiscreteKeysStates(),
	m_fDeltaTime(0.0f),
	m_fLastFrame(0.0f),
	m_fCurrentFrame(0.0f)
{
	InitEngine();
}

Engine::~Engine()
{
	delete m_pMainWindow;
	glfwTerminate();
}

Engine * Engine::GetGlobalEngine()
{
	assert(Engine::sm_pEngine);
	return Engine::sm_pEngine;
}

void Engine::InitEngine()
{
	assert(sm_pEngine == nullptr); // protection from multiple calls

	m_pMainWindow = MakeWindow(1280, 720, "VISSA");
	m_pMainWindow->SetAsCurrentContext();

	m_tRenderer.InitRenderer();
	glAssert();
	m_tCamera.SetToPosition(glm::vec3(0.0f, 0.0f, 1500.0f));
	m_tGUI.InitForWindow(*m_pMainWindow);
	// scene is alreay ready (but empty)
	// collision detection not called when scene is empty

	Engine::sm_pEngine = this;	// globally accesible pointer for callback functions
}

Window* Engine::MakeWindow(int32_t iWindowWidth, int32_t iWindowHeight, std::string sWindowName, Window* pContextSharingWindow)
{
	

	// Load GLFW
	glfwSetErrorCallback(Engine::GLFWErrorCallBack);
	glfwInit();

	GLFWwindow* pPreviousContext = glfwGetCurrentContext();

	// create window and GL context
	Window* pNewWindow = new Window(iWindowWidth, iWindowHeight, sWindowName, pContextSharingWindow);
	pNewWindow->SetAsCurrentContext();	// for GLAD loading if it is the first window. actual context handling is done outside

	// Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		assert(!"Failed to initialize GLAD");
	}

	glViewport(0, 0, pNewWindow->m_iWindowWidth, pNewWindow->m_iWindowHeight);
	glfwSetFramebufferSizeCallback(pNewWindow->m_pGLFWwindow, Engine::ResizeWindowCallback);
	glfwSetCursorPosCallback(pNewWindow->m_pGLFWwindow, Engine::MouseMoveCallBack);
	glfwSetScrollCallback(pNewWindow->m_pGLFWwindow, Engine::MouseScrollCallBack);
	glfwSetMouseButtonCallback(pNewWindow->m_pGLFWwindow, Engine::MouseClickCallBack);

	pNewWindow->m_iMouseShowingStack = 1;

	// mark as ready
	pNewWindow->m_bIsInitialized = true;

	// reset to previous context
	glfwMakeContextCurrent(pPreviousContext);

	return pNewWindow;
}

void Engine::MainLoop()
{
	while (!m_pMainWindow->WindowShouldClose())
	{
		// delta time
		float m_fCurrentFrame = static_cast<float>(glfwGetTime()); // float is good enough 
		m_fDeltaTime = m_fCurrentFrame - m_fLastFrame;
		m_fLastFrame = m_fCurrentFrame;

		// input
		glfwPollEvents();	// ask GLFW what kind of input happened
		ProcessKeyboardInput();

		// update
		m_tVisualization.Update(m_fDeltaTime);

		// rendering
		// 3D
		m_tRenderer.Render(m_tCamera, *m_pMainWindow, m_tVisualization);
		// GUI
		m_tGUI.Render(*this);

		// glfw: swap buffers
		glfwSwapBuffers(m_pMainWindow->m_pGLFWwindow);
	}
}

Window * Engine::GetMainWindow()
{
	assert(m_pMainWindow);

	return m_pMainWindow;
}

void Engine::ProcessKeyboardInput()
{
	/*
		only reacting to key presses that do not concern the UI.
	*/
	if (!ImGui::GetIO().WantCaptureKeyboard)
	{

		/*
			generally available inputs
		*/
		{
			// discrete, non-continuous inputs
			{
				if (IsDiscreteKeyReady(GLFW_KEY_ESCAPE))
				{
					m_tGUI.ShowMenu(!m_tGUI.IsMenuActive());
					m_pMainWindow->SetHardCaptureMouse(!m_tGUI.IsMenuActive());
					m_tGUI.SetCaptureMouse(m_tGUI.IsMenuActive());	// the GUI captures mouse input when the menu is active
				}

				if (IsDiscreteKeyReady(GLFW_KEY_H))
				{
					m_tGUI.ToggleHelpWindow();
				}
			}

			// camera control
			{
				Camera& rCamera = m_tCamera;

				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_W) == GLFW_PRESS)
					rCamera.ProcessKeyboard(FORWARD, m_fDeltaTime);
				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_S) == GLFW_PRESS)
					rCamera.ProcessKeyboard(BACKWARD, m_fDeltaTime);
				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_A) == GLFW_PRESS)
					rCamera.ProcessKeyboard(LEFT, m_fDeltaTime);
				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_D) == GLFW_PRESS)
					rCamera.ProcessKeyboard(RIGHT, m_fDeltaTime);

				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_Q) == GLFW_PRESS)
					rCamera.ProcessKeyboard(UP, m_fDeltaTime);
				if (glfwGetKey(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_E) == GLFW_PRESS)
					rCamera.ProcessKeyboard(DOWN, m_fDeltaTime);
			}
		}

		/*
			inputs that are only available when the menu is not active and the scene is running
		*/
		if (!m_tGUI.IsMenuActive())
		{
			/*
				generally available inputs when the scene is running
			*/
			{
				int iCurrentKey = GLFW_KEY_M;

				if (IsDiscreteKeyReady(iCurrentKey))
				{
					m_pMainWindow->SetHardCaptureMouse(!m_pMainWindow->IsMouseCaptured());	// toggle between captured mouse or a cursor
					m_tGUI.SetCaptureMouse(!m_pMainWindow->IsMouseCaptured());			// control GUI behaviour
				}
			}

			/*
				inputs that are only available when the mouse is captured/the camera controlled
			*/
			if (m_pMainWindow->IsMouseCaptured())
			{
				

			}
			else // inputs that are only available when the mouse is freely moving in an active scene
			{
				// empty
			}
		}
	}
}

bool Engine::IsDiscreteKeyReady(int iKey)
{
	const int iKeyState = glfwGetKey(m_pMainWindow->m_pGLFWwindow, iKey);

	// this translates to: if a key is pressed this frame AND was not pressed the frame before
	bool bResult = (iKeyState && m_iDiscreteKeysStates[iKey] == GLFW_RELEASE);

	// remember the current state for the next time we check it (which very well could be the very next frame)
	m_iDiscreteKeysStates[iKey] = iKeyState;

	return bResult;
}

void Engine::ResizeWindowCallback(GLFWwindow* pWindow, int iNewWidth, int iNewHeight)
{
	Engine* pEngine = GetGlobalEngine();

	// TODO: this is not clean. needs proper handling on window per window basis
	if (pWindow == pEngine->m_pMainWindow->m_pGLFWwindow)
	{
		pEngine->m_pMainWindow->m_iWindowWidth = iNewWidth;
		pEngine->m_pMainWindow->m_iWindowHeight = iNewHeight;
		glViewport(0, 0, iNewWidth, iNewHeight);
	}
}

void Engine::MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset)
{
	Engine* pEngine = GetGlobalEngine();

	// scrolling = zooming and disabled for now
	//pEngine->m_tCamera.ProcessMouseScroll(fYOffset);
}

void Engine::MouseMoveCallBack(GLFWwindow* pWindow, double dXPosition, double dYPosition)
{
	Engine* pEngine = GetGlobalEngine();

	if (pWindow == pEngine->m_pMainWindow->m_pGLFWwindow) // if input is for main window, proceed
	{
		/*
			only reacting to mouse moving that does not concern the GUI
		*/
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			const float fXPosition = static_cast<float>(dXPosition);
			const float fYPosition = static_cast<float>(dYPosition);

			/*
				inputs that are available only when the scene is active
			*/
			if (!pEngine->m_tGUI.IsMenuActive())
			{
				Window& rWindow = *(pEngine->m_pMainWindow);
				if (rWindow.IsMouseCaptured()) // Control the camera only when mouse is captured
				{
					if (rWindow.m_bFirstMouse)
					{
						rWindow.m_fLastXOfMouse = fXPosition;
						rWindow.m_fLastYOfMouse = fYPosition;
						rWindow.m_bFirstMouse = false;
					}

					float xoffset = fXPosition - rWindow.m_fLastXOfMouse;
					float yoffset = rWindow.m_fLastYOfMouse - fYPosition; // reversed since y-coordinates go from bottom to top

					rWindow.m_fLastXOfMouse = fXPosition;
					rWindow.m_fLastYOfMouse = fYPosition;

					Camera& rCamera = pEngine->m_tCamera;

					rCamera.ProcessMouseMovement(xoffset, yoffset);


				}
				else	// reacting to mouse movement when the camera is not active
				{
					// empty
				}
			}
		}
	}
}

void Engine::MouseClickCallBack(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	Engine* pEngine = Engine::GetGlobalEngine();

	if (pWindow == pEngine->m_pMainWindow->m_pGLFWwindow) // if input is for main window, proceed
	{
		/*
			reacting only to clicks that do not concern the UI
		*/
		if (!ImGui::GetIO().WantCaptureMouse)
		{
			GUI& rGUI = pEngine->m_tGUI;
			if (!rGUI.IsMenuActive()) // scene only reacting to clicks when the menu is not active
			{
				if (iButton == GLFW_MOUSE_BUTTON_LEFT && iAction == GLFW_PRESS) // single click of left mouse button
				{
					// TODO: yes, there is code duplication going on here. however, the entire scope of how clicks are handled is unclear. that is why improving this might be pointless

					Window& rWindow = *(pEngine->m_pMainWindow);
					if (rWindow.IsMouseCaptured())	// reacting to clicks made while camera control is active
					{
						// construct ray
						CollisionDetection::Ray tRay(pEngine->m_tCamera.GetCurrentPosition(), pEngine->m_tCamera.GetFrontVector());

						// check for intersections with ray
						//CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::CastRayIntoBVH(pEngine->m_tVisualization.m_tTopDownBVH_AABB, tRay);
						CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::BruteForceRayIntoObjects(pEngine->m_tVisualization.m_vecObjects, tRay);

						SceneObject* pPreviouslyFocusedObject = pEngine->m_tGUI.GetFocusedObject();
						if (pPreviouslyFocusedObject)
							pEngine->m_tGUI.CancelObjectPropertiesChanges();

						// execute orders in accordance with the result
						if (tResult.IntersectionWithObjectOccured())
						{
							pEngine->m_tGUI.SetFocusedObject(tResult.m_pFirstIntersectedSceneObject);
							pEngine->m_tGUI.SetObjectPropertiesWindowPosition(static_cast<float>(pEngine->m_pMainWindow->m_iWindowWidth * 0.5f), static_cast<float>(pEngine->m_pMainWindow->m_iWindowHeight *0.5f)); // center of window						
							pEngine->m_tGUI.ShowObjectPropertiesWindow(true);
						}
						else
						{

							pEngine->m_tGUI.SetFocusedObject(nullptr); // no object in focus now
						}
					}
					else	// reacting to clicks of a freely moving cursor
					{
						// extra step: the freely moving cursor position is, in world space, not aligned with the camera position. needs adjustment!
						const glm::vec3 vec3RayDirection = pEngine->m_tRenderer.ConstructRayDirectionFromMousePosition(rWindow);

						// construct ray
						CollisionDetection::Ray tRay(pEngine->m_tCamera.GetCurrentPosition(), vec3RayDirection);

						// check for intersections with ray
						//CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::CastRayIntoBVH(pEngine->m_tVisualization.m_tTopDownBVH_AABB, tRay);
						CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::BruteForceRayIntoObjects(pEngine->m_tVisualization.m_vecObjects, tRay);

						SceneObject* pPreviouslyFocusedObject = pEngine->m_tGUI.GetFocusedObject();
						if (pPreviouslyFocusedObject)
							pEngine->m_tGUI.CancelObjectPropertiesChanges();

						// execute orders in accordance with the result
						if (tResult.IntersectionWithObjectOccured())
						{
							pEngine->m_tGUI.SetFocusedObject(tResult.m_pFirstIntersectedSceneObject);
							Window::MousePositionInWindow tCurrentMousePosition = pEngine->m_pMainWindow->GetCurrentMousePosition();
							pEngine->m_tGUI.SetObjectPropertiesWindowPosition(tCurrentMousePosition.m_fXPosition, tCurrentMousePosition.m_fYPosition); // center of window						
							pEngine->m_tGUI.ShowObjectPropertiesWindow(true);
						}
						else
						{
							pEngine->m_tGUI.SetFocusedObject(nullptr); // no object in focus now
						}
					}
				}
			}
		}
	}
}

void Engine::GLFWErrorCallBack(int iError, const char * sDescription)
{
	fprintf(stderr, "Glfw Error %d: %s\n", iError, sDescription);
}