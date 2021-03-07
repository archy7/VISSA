#include "Window.h"

#include <assert.h>
#include <stdio.h>
#include "Camera.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

GLFWWindowAndOjbectTuple Window::GLFWwindowLookUpTuple;

Window::Window():
	m_pGLFWwindow(nullptr),
	m_iWindowWidth(1280),
	m_iWindowHeight(720),
	m_vec4fClearColor(),
	m_fLastXOfMouse(m_iWindowWidth/ 2.0f),
	m_fLastYOfMouse(m_iWindowHeight / 2.0f),
	m_iDiscreteKeysStates(),
	m_bFirstMouse(true),
	m_bMouseCaptured(true),
	m_bMenuActive(false),
	m_bIsInitialized(false)
{
	m_vec4fClearColor[0] = 0.3f; // red;
	m_vec4fClearColor[1] = 0.3f; // green;
	m_vec4fClearColor[2] = 0.3f; // blue;
	m_vec4fClearColor[3] = 1.0f; // alpha;
}

Window::~Window()
{
	
}

void Window::InitWindow()
{
	RecreateWindow();

	// Load GL functions
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		assert(!"Failed to initialize GLAD");
	}

	InitGUI();

	glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
	glfwSetFramebufferSizeCallback(m_pGLFWwindow, ResizeWindowCallback);
	glfwSetCursorPosCallback(m_pGLFWwindow, MouseMoveCallBack);
	glfwSetScrollCallback(m_pGLFWwindow, MouseScrollCallBack);

	SetMouseCaptureMode(m_bMouseCaptured);

	

	// populate the lookup tuple
	Window::GLFWwindowLookUpTuple.m_pGLFWWindow = m_pGLFWwindow;
	Window::GLFWwindowLookUpTuple.m_pAssociatedWindowObject = this;

	// mark as ready
	m_bIsInitialized = true;
}

void Window::InitGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_pGLFWwindow, true);
	ImGui_ImplOpenGL3_Init("#version 430");
}

void Window::CleanUpSequence()
{
	glfwDestroyWindow(m_pGLFWwindow);
	glfwTerminate();
}

inline void Window::ReInitializationSequence()
{
	assert(!"TODO: Window::ReInit()");
}

void Window::ProcessInput(GLFWwindow * pWindow, float fDeltaTime)
{
	/*
		only reacting to key presses that do not concern the UI.
	*/
	if (!ImGui::GetIO().WantCaptureKeyboard)
	{
		Window* pAssociatedWindow = GetWindowForGLFWwindow(pWindow);
		assert(pAssociatedWindow);

		/*
			generally avaiable inputs
		*/
		{
			if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
				glfwSetWindowShouldClose(pWindow, true);

			// discrete, non-continuous inputs
			{	
				int iCurrentKey = GLFW_KEY_P;
				
				if (pAssociatedWindow->IsDiscreteKeyReady(iCurrentKey))
				{
					pAssociatedWindow->m_bMenuActive = !pAssociatedWindow->m_bMenuActive;		// flip the menu active flag
					pAssociatedWindow->SetMouseCaptureMode(!pAssociatedWindow->m_bMenuActive);	// mouse is captured when menu active and vice versa
				}
			}
		}

		/*
			inputs that are only available when the menu is not active and the scene is running
		*/
		if (!pAssociatedWindow->m_bMenuActive)
		{
			/*
				inputs that are only available when the mouse is captured/the camera controlled
			*/
			if (pAssociatedWindow->m_bMouseCaptured)
			{
				Camera* pAssociatedCamera = GetCameraForGLFWwindow(pWindow);
				assert(pAssociatedCamera);

				if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
					pAssociatedCamera->ProcessKeyboard(FORWARD, fDeltaTime);
				if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
					pAssociatedCamera->ProcessKeyboard(BACKWARD, fDeltaTime);
				if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
					pAssociatedCamera->ProcessKeyboard(LEFT, fDeltaTime);
				if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
					pAssociatedCamera->ProcessKeyboard(RIGHT, fDeltaTime);
			}
			else // inputs that are only available when the mouse is freely moving in an active scene
			{
				// empty
			}
		}
	}
}

Window * Window::GetWindowForGLFWwindow(GLFWwindow * pGivenWindow)
{
	assert(pGivenWindow);
	assert(pGivenWindow == GLFWwindowLookUpTuple.m_pGLFWWindow);
	// quick, dirty, gets the job done. Can be expanded though.
	return GLFWwindowLookUpTuple.m_pAssociatedWindowObject;
}

Camera * Window::GetCameraForGLFWwindow(GLFWwindow * pGivenWindow)
{
	assert(pGivenWindow);
	assert(pGivenWindow == GLFWwindowLookUpTuple.m_pGLFWWindow);
	// quick, dirty, gets the job done. Can be expanded though.
	return GLFWwindowLookUpTuple.m_pAssociatedCamera;
}

void Window::ResizeWindowCallback(GLFWwindow* pWindow, int iNewWidth, int iNewHeight)
{
	Window* pAssociatedWindow = GetWindowForGLFWwindow(pWindow);
	assert(pAssociatedWindow);

	pAssociatedWindow->m_iWindowWidth = iNewWidth;
	pAssociatedWindow->m_iWindowHeight = iNewHeight;
	glViewport(0, 0, iNewWidth, iNewHeight);
}

void Window::MouseScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset)
{
	Camera* pAssociatedCamera = GetCameraForGLFWwindow(pWindow);
	assert(pAssociatedCamera);

	// scrolling = zooming and disabled for now
	// pAssociatedCamera->ProcessMouseScroll(fYOffset);
}

void Window::MouseMoveCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition)
{
	/*
		only reacting to mouse moving that does not concern the GUI
	*/
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		Window* pAssociatedWindow = GetWindowForGLFWwindow(pWindow);
		assert(pAssociatedWindow);

		/*
			inputs that are available only when the scene is active
		*/
		if (!pAssociatedWindow->m_bMenuActive)
		{
			if (pAssociatedWindow->m_bMouseCaptured) // Control the camera only when mouse is captured
			{
				if (pAssociatedWindow->m_bFirstMouse)
				{
					pAssociatedWindow->m_fLastXOfMouse = fXPosition;
					pAssociatedWindow->m_fLastYOfMouse = fYPosition;
					pAssociatedWindow->m_bFirstMouse = false;
				}

				float xoffset = fXPosition - pAssociatedWindow->m_fLastXOfMouse;
				float yoffset = pAssociatedWindow->m_fLastYOfMouse - fYPosition; // reversed since y-coordinates go from bottom to top

				pAssociatedWindow->m_fLastXOfMouse = fXPosition;
				pAssociatedWindow->m_fLastYOfMouse = fYPosition;

				Camera* pAssociatedCamera = GetCameraForGLFWwindow(pWindow);
				assert(pAssociatedCamera);

				pAssociatedCamera->ProcessMouseMovement(xoffset, yoffset);
			}
			else	// reacting to mouse movement when the camera is not active
			{
				// empty
			}
		}
	}
}

void Window::MouseClickCallBack(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	/*
		reacting only to clicks that do not concern the UI
	*/
	if (!ImGui::GetIO().WantCaptureMouse)
	{
		Window* pAssociatedWindow = GetWindowForGLFWwindow(pWindow);
		assert(pAssociatedWindow);

		if(!pAssociatedWindow->m_bMenuActive) // scene only reacting to clicks when the menu is not active
		{ 
			if (pAssociatedWindow->m_bMouseCaptured)	// reacting to clicks made while the camera is active
			{
				
			}
			else	// reacting to clicks of a freely moving cursor
			{

			}
		}
	}
}

void Window::GLFWErrorCallBack(int iError, const char * sDescription)
{
	fprintf(stderr, "Glfw Error %d: %s\n", iError, sDescription);
}

void Window::RecreateWindow()
{
	assert(m_iWindowWidth > 0);
	assert(m_iWindowHeight > 0);

	glfwSetErrorCallback(GLFWErrorCallBack);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif


	m_pGLFWwindow = glfwCreateWindow(m_iWindowWidth, m_iWindowHeight, "VISSA", NULL, NULL);
	if (m_pGLFWwindow == NULL)
	{
		assert(!"Failed to create GFLW window\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_pGLFWwindow);
}

int Window::WindowShouldClose()
{
	return glfwWindowShouldClose(m_pGLFWwindow);
}

void Window::SetMouseCaptureMode(bool bIsCaptured)
{
	m_bMouseCaptured = bIsCaptured;
	if(m_bMouseCaptured)
	{
		/**
			tell GLFW to capture our mouse.
			This is for when the camera is controlled
		*/
		glfwSetInputMode(m_pGLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		m_bFirstMouse = true;
	}
	else
	{
		/*
			tell GLFW to show the mouse and let it move freely.
			this is for interactions with the UI and when the mouse is allowed to move freely on the screen, i.e. for hovering and clicking objects.
		*/
		glfwSetInputMode(m_pGLFWwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

bool Window::IsDiscreteKeyReady(int iKey)
{
	const int iKeyState = glfwGetKey(m_pGLFWwindow, iKey);

	// this translates to: if a key is pressed this frame AND was not pressed the frame before
	bool bResult = (iKeyState && m_iDiscreteKeysStates[iKey] == GLFW_RELEASE);

	// remember the current state for the next time we check it (which very well could be the very next frame)
	m_iDiscreteKeysStates[iKey] = iKeyState;

	return bResult;
}