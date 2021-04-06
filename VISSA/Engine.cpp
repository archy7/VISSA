#include "Engine.h"

#include "SceneObject.h"

Engine* Engine::sm_pEngine = nullptr;

Engine::Engine() :
	m_tWindow(),
	m_tRenderer(),
	m_tCamera(glm::vec3(0.0f, 100.0f, 400.0f)),
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
	glfwTerminate();
}

Engine * Engine::GetGlobalEngine()
{
	assert(Engine::sm_pEngine);
	return Engine::sm_pEngine;
}

void Engine::InitEngine()
{
	m_tWindow.InitWindow();
	m_tRenderer.InitRenderer();
	glAssert();
	// camera is already ready
	m_tGUI.InitForWindow(m_tWindow);
	// scene is alreay ready (but empty)
	// collision detection not called when scene is empty

	Engine::sm_pEngine = this;	// globally accesible pointer for callback functions
}

void Engine::MainLoop()
{
	while (!m_tWindow.WindowShouldClose())
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
		m_tRenderer.Render(m_tCamera, m_tWindow, m_tVisualization);
		// GUI
		m_tGUI.Render(*this);

		// glfw: swap buffers
		glfwSwapBuffers(m_tWindow.m_pGLFWwindow);
	}
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
					m_tWindow.SetHardCaptureMouse(!m_tGUI.IsMenuActive());
					m_tGUI.SetCaptureMouse(m_tGUI.IsMenuActive());	// the GUI captures mouse input when the menu is active
				}

				if (IsDiscreteKeyReady(GLFW_KEY_H))
				{
					m_tGUI.ToggleHelpWindow();
				}
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
					m_tWindow.SetHardCaptureMouse(!m_tWindow.IsMouseCaptured());	// toggle between captured mouse or a cursor
					m_tGUI.SetCaptureMouse(!m_tWindow.IsMouseCaptured());			// control GUI behaviour
				}
			}

			/*
				inputs that are only available when the mouse is captured/the camera controlled
			*/
			if (m_tWindow.IsMouseCaptured())
			{
				Camera& rCamera = m_tCamera;

				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_W) == GLFW_PRESS)
					rCamera.ProcessKeyboard(FORWARD, m_fDeltaTime);
				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_S) == GLFW_PRESS)
					rCamera.ProcessKeyboard(BACKWARD, m_fDeltaTime);
				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_A) == GLFW_PRESS)
					rCamera.ProcessKeyboard(LEFT, m_fDeltaTime);
				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_D) == GLFW_PRESS)
					rCamera.ProcessKeyboard(RIGHT, m_fDeltaTime);

				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_Q) == GLFW_PRESS)
					rCamera.ProcessKeyboard(UP, m_fDeltaTime);
				if (glfwGetKey(m_tWindow.m_pGLFWwindow, GLFW_KEY_E) == GLFW_PRESS)
					rCamera.ProcessKeyboard(DOWN, m_fDeltaTime);

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
	const int iKeyState = glfwGetKey(m_tWindow.m_pGLFWwindow, iKey);

	// this translates to: if a key is pressed this frame AND was not pressed the frame before
	bool bResult = (iKeyState && m_iDiscreteKeysStates[iKey] == GLFW_RELEASE);

	// remember the current state for the next time we check it (which very well could be the very next frame)
	m_iDiscreteKeysStates[iKey] = iKeyState;

	return bResult;
}

void Engine::ResizeWindowCallback(GLFWwindow* pWindow, int iNewWidth, int iNewHeight)
{
	Engine* pEngine = GetGlobalEngine();

	pEngine->m_tWindow.m_iWindowWidth = iNewWidth;
	pEngine->m_tWindow.m_iWindowHeight = iNewHeight;
	glViewport(0, 0, iNewWidth, iNewHeight);
}

void Engine::MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset)
{
	Engine* pEngine = GetGlobalEngine();

	// scrolling = zooming and disabled for now
	//pEngine->m_tCamera.ProcessMouseScroll(fYOffset);
}

void Engine::MouseMoveCallBack(GLFWwindow* pWindow, double dXPosition, double dYPosition)
{
	/*
		only reacting to mouse moving that does not concern the GUI
	*/
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		Engine* pEngine = GetGlobalEngine();
		const float fXPosition = static_cast<float>(dXPosition);
		const float fYPosition = static_cast<float>(dYPosition);

		/*
			inputs that are available only when the scene is active
		*/
		if (!pEngine->m_tGUI.IsMenuActive())
		{
			Window& rWindow = pEngine->m_tWindow;
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

void Engine::MouseClickCallBack(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	/*
		reacting only to clicks that do not concern the UI
	*/
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		Engine* pEngine = Engine::GetGlobalEngine();

		GUI& rGUI = pEngine->m_tGUI;
		if (!rGUI.IsMenuActive()) // scene only reacting to clicks when the menu is not active
		{
			if (iButton == GLFW_MOUSE_BUTTON_LEFT && iAction == GLFW_PRESS) // single click of left mouse button
			{
				// TODO: yes, there is code duplication going on here. however, the entire scope of how clicks are handled is unclear. that is why improving this might be pointless

				Window& rWindow = pEngine->m_tWindow;
				if (rWindow.IsMouseCaptured())	// reacting to clicks made while camera control is active
				{
					// construct ray
					CollisionDetection::Ray tRay(pEngine->m_tCamera.GetCurrentPosition(), pEngine->m_tCamera.GetFrontVector());

					// check for intersections with ray
					//CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::CastRayIntoBVH(pEngine->m_tVisualization.m_tTopDownBVH_AABB, tRay);
					CollisionDetection::RayCastIntersectionResult tResult = CollisionDetection::BruteForceRayIntoObjects(pEngine->m_tVisualization.m_vecObjects, tRay);

					// execute orders in accordance with the result
					if (tResult.IntersectionWithObjectOccured())
					{
						pEngine->m_tVisualization.SetFocusedObject(tResult.m_pFirstIntersectedSceneObject);
						pEngine->m_tGUI.SetObjectPropertiesWindowPosition(static_cast<float>(pEngine->m_tWindow.m_iWindowWidth * 0.5f), static_cast<float>(pEngine->m_tWindow.m_iWindowHeight *0.5f)); // center of window						
						pEngine->m_tGUI.ShowObjectPropertiesWindow(true);
					}
					else
					{
						pEngine->m_tGUI.ShowObjectPropertiesWindow(false);
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

					// execute orders in accordance with the result
					if (tResult.IntersectionWithObjectOccured())
					{
						pEngine->m_tVisualization.SetFocusedObject(tResult.m_pFirstIntersectedSceneObject);
						Window::MousePositionInWindow tCurrentMousePosition = pEngine->m_tWindow.GetCurrentMousePosition();
						pEngine->m_tGUI.SetObjectPropertiesWindowPosition(tCurrentMousePosition.m_fXPosition, tCurrentMousePosition.m_fYPosition); // center of window						
						pEngine->m_tGUI.ShowObjectPropertiesWindow(true);
					}
					else
					{
						pEngine->m_tGUI.ShowObjectPropertiesWindow(false);
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