#include "Window.h"

#include <assert.h>
#include "Camera.h"

GLFWWindowAndOjbectTuple Window::GLFWwindowLookUpTable[];

Window::Window():
	m_pGLFWwindow(nullptr),
	m_iWindowWidth(1280),
	m_iWindowHeight(720),
	m_fLastXOfMouse(m_iWindowWidth/ 2.0f),
	m_fLastYOfMouse(m_iWindowHeight / 2.0f),
	m_bFirstMouse(true),
	m_bIsInitialized(false)
{

}

Window::~Window()
{

}

void Window::InitWindow()
{
	RecreateWindow();

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		assert(!"Failed to initialize GLAD");
	}

	glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
	glfwSetFramebufferSizeCallback(m_pGLFWwindow, ResizeWindowCallback);
	glfwSetCursorPosCallback(m_pGLFWwindow, MouseCallBack);
	glfwSetScrollCallback(m_pGLFWwindow, ScrollCallBack);

	// tell GLFW to capture our mouse
	glfwSetInputMode(m_pGLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// populate the lookup table
	Window::GLFWwindowLookUpTable[0].m_pGLFWWindow = m_pGLFWwindow;
	Window::GLFWwindowLookUpTable[0].m_pAssociatedWindowObject = this;

	// mark as ready
	m_bIsInitialized = true;
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
	// this is a quick and dirty solution and not a proper design pattern.
	Camera* pAssociatedCamera = Window::GLFWwindowLookUpTable[0].m_pAssociatedCamera;
	assert(pAssociatedCamera);

	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);

	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		pAssociatedCamera->ProcessKeyboard(FORWARD, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		pAssociatedCamera->ProcessKeyboard(BACKWARD, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		pAssociatedCamera->ProcessKeyboard(LEFT, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		pAssociatedCamera->ProcessKeyboard(RIGHT, fDeltaTime);
}

void Window::ResizeWindowCallback(GLFWwindow* pWindow, int iNewWidth, int iNewHeight)
{
	// this is a quick and dirty solution and not a proper design pattern.
	Window* pAssociatedWindow = Window::GLFWwindowLookUpTable[0].m_pAssociatedWindowObject;
	assert(pAssociatedWindow);

	pAssociatedWindow->m_iWindowWidth = iNewWidth;
	pAssociatedWindow->m_iWindowHeight = iNewHeight;
	glViewport(0, 0, iNewWidth, iNewHeight);
}

void Window::ScrollCallBack(GLFWwindow* pWindow, double fXOffset, double fYOffset)
{
	// this is a quick and dirty solution and not a proper design pattern.
	Camera* pAssociatedCamera = Window::GLFWwindowLookUpTable[0].m_pAssociatedCamera;
	assert(pAssociatedCamera);

	pAssociatedCamera->ProcessMouseScroll(fYOffset);
}

void Window::MouseCallBack(GLFWwindow* pWindow, double fXPosition, double fYPosition)
{
	// this is a quick and dirty solution and not a proper design pattern.
	Window* pAssociatedWindow = Window::GLFWwindowLookUpTable[0].m_pAssociatedWindowObject;
	assert(pAssociatedWindow);

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

	// this is a quick and dirty solution and not a proper design pattern.
	Camera* pAssociatedCamera = Window::GLFWwindowLookUpTable[0].m_pAssociatedCamera;
	assert(pAssociatedCamera);

	pAssociatedCamera->ProcessMouseMovement(xoffset, yoffset);
}

void Window::RecreateWindow()
{
	assert(m_iWindowWidth > 0);
	assert(m_iWindowHeight > 0);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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