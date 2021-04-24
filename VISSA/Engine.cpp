#include "Engine.h"

#include "SceneObject.h"

Engine* Engine::sm_pEngine = nullptr;

Engine::Engine() :
	m_pMainWindow(nullptr),
	m_tRenderer(),
	m_tGUI(),
	m_pVisualization(nullptr),
	m_iDiscreteKeysStates(),
	m_fDeltaTime(0.0f),
	m_fLastFrame(0.0f),
	m_fCurrentFrame(0.0f)
{
	InitEngine();
}

Engine::~Engine()
{
	delete m_pVisualization;
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
	Engine::sm_pEngine = this;	// globally accesible pointer for callback functions

	// Load GLFW
	glfwSetErrorCallback(Engine::GLFWErrorCallBack);
	glfwInit();

	m_pMainWindow = MakeWindowAndSetAsContext(1280, 720, "VISSA");
	glAssert();
	m_pMainWindow->SetAsCurrentRenderContext();

	m_tRenderer.InitRenderer();
	glAssert();
	GUI::InitForWindow(*m_pMainWindow);
}

Window* Engine::MakeWindowAndSetAsContext(int32_t iWindowWidth, int32_t iWindowHeight, std::string sWindowName)
{
	//GLFWwindow* pPreviousContext = glfwGetCurrentContext();

	// create window and GL context
	Window* pNewWindow;
	if(!Engine::MainWindowCreated())
		pNewWindow = new Window(iWindowWidth, iWindowHeight, sWindowName, nullptr);
	else
		pNewWindow = new Window(iWindowWidth, iWindowHeight, sWindowName, Engine::GetGlobalEngine()->GetMainWindow()->m_pGLFWwindow);
	glfwMakeContextCurrent(pNewWindow->m_pGLFWwindow); // for GLAD loading if it is the first window. actual context handling is done outside

	// Load GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		assert(!"Failed to initialize GLAD");
	}

#ifdef _DEBUG
	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Renderer::glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif // _DEBUG

	

	glViewport(0, 0, pNewWindow->m_iWindowWidth, pNewWindow->m_iWindowHeight);
	glfwSetFramebufferSizeCallback(pNewWindow->m_pGLFWwindow, Engine::ResizeWindowCallback);
	glfwSetCursorPosCallback(pNewWindow->m_pGLFWwindow, Engine::MouseMoveCallBack);
	glfwSetScrollCallback(pNewWindow->m_pGLFWwindow, Engine::MouseScrollCallBack);
	glfwSetMouseButtonCallback(pNewWindow->m_pGLFWwindow, Engine::MouseClickCallBack);
	glAssert();

	pNewWindow->m_iMouseShowingStack = 1;
	glAssert();

	// mark as ready
	pNewWindow->m_bIsInitialized = true;
	glAssert();

	// reset to previous context
	/*glfwMakeContextCurrent(pPreviousContext);
	glAssert();*/

	return pNewWindow;
}

bool Engine::MainWindowCreated()
{
	return Engine::GetGlobalEngine()->m_pMainWindow;
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
		if(m_pVisualization)
			m_pVisualization->Update(m_fDeltaTime);

		// main window
		// 3D
		m_tRenderer.RenderIntoMainWindow(*m_pMainWindow);
		// GUI
		m_pMainWindow->SetAsCurrentRenderContext();
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		
		// visualization
		if (m_pVisualization)
			m_pVisualization->Render();
		
		m_tGUI.Render(*this);

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers
		glfwSwapBuffers(m_pMainWindow->m_pGLFWwindow);
	}
}

Window * Engine::GetMainWindow()
{
	assert(Engine::GetGlobalEngine()->m_pMainWindow);

	return Engine::GetGlobalEngine()->m_pMainWindow;
}

void Engine::ProcessKeyboardInput()
{
	/*
		only reacting to key presses that do not concern the UI.
	*/
	if (!ImGui::GetIO().WantCaptureKeyboard)
	{

		/*
			inputs for the engine itself
		*/
		{
			// discrete, non-continuous inputs
			{
				if (IsDiscreteKeyReadyForWindow(m_pMainWindow->m_pGLFWwindow, GLFW_KEY_ESCAPE))
				{
					if (m_pVisualization) // only when a vis loaded, can the main menu be closed with ESC
					{
						m_tGUI.ShowMenu(!m_tGUI.IsMenuActive());
						m_pMainWindow->SetHardCaptureMouse(!m_tGUI.IsMenuActive());
						m_tGUI.SetCaptureMouse(m_tGUI.IsMenuActive());	// the GUI captures mouse input when the menu is active
					}
				}
			}

			// continuous inputs
			{
				// empty
			}
		}

		/*
			every other keyboard input will be handled by the current visualization
		*/
		if (m_pVisualization)
		{
			m_pVisualization->ProcessKeyboardInput();
		}
	}
}

bool Engine::IsDiscreteKeyReadyForWindow(GLFWwindow* pWindow, int iKey)
{
	assert(Engine::GetMainWindow()->m_pGLFWwindow == pWindow);	// if you land here, it is becuase only the main window supports discrete keys as of now. TODO!

	Engine* pEngine =  Engine::GetGlobalEngine();

	const int iKeyState = glfwGetKey(pEngine->m_pMainWindow->m_pGLFWwindow, iKey);

	// this translates to: if a key is pressed this frame AND was not pressed the frame before
	bool bResult = (iKeyState && pEngine->m_iDiscreteKeysStates[iKey] == GLFW_RELEASE);

	// remember the current state for the next time we check it (which very well could be the very next frame)
	pEngine->m_iDiscreteKeysStates[iKey] = iKeyState;

	return bResult;
}

void Engine::ResizeWindowCallback(GLFWwindow* pWindow, int iNewWidth, int iNewHeight)
{
	assert(!"window resizing currently not accounted for and is disabled in general. You should not land here.");

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
	/*
		only reacting to mouse moving that does not concern the GUI
	*/
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		Engine* pEngine = Engine::GetGlobalEngine();

		/*
			inputs for the engine itself
		*/
		{
			// empty
		}

		// also forwarding the mouse move to the current visualization to handle it
		if (pEngine->m_pVisualization)
			pEngine->m_pVisualization->ProcessMouseMovement(pWindow, dXPosition, dYPosition);
	}
}

void Engine::MouseClickCallBack(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	Engine* pEngine = Engine::GetGlobalEngine();

	if (!ImGui::GetIO().WantCaptureMouse)
	{
		/*
			inputs for the engine itself
		*/
		{
			// empty
		}

		// also forwarding the mouse click to the current visualization to handle it
		if (pEngine->m_pVisualization)
			pEngine->m_pVisualization->ProcessMouseClick(pWindow, iButton, iAction, iModifiers);
	}
}

void Engine::GLFWErrorCallBack(int iError, const char * sDescription)
{
	fprintf(stderr, "Glfw Error %d: %s\n", iError, sDescription);
}