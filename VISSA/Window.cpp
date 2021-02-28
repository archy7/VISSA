#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <assert.h>

GLFWwindow* Window::pWindow = nullptr;
int Window::iWindowWidth = 800;
int Window::iWindowHeight = 600;

void Window::InitializationSequence()
{
	RecreateWindow();

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		assert(!"Failed to initialize GLAD");
	}

	glViewport(0, 0, iWindowWidth, iWindowHeight);
	glfwSetFramebufferSizeCallback(pWindow, ResizeWindowCallback);
}

void Window::CleanUpSequence()
{
	glfwDestroyWindow(pWindow);
	glfwTerminate();
}

inline void Window::ReInitializationSequence()
{
	assert(!"TODO: Window::ReInit()");
}

void Window::ResizeWindowCallback(GLFWwindow* window, int iNewWidth, int iNewHeight)
{
	iWindowWidth = iNewWidth;
	iWindowHeight = iNewHeight;
	glViewport(0, 0, iWindowWidth, iWindowHeight);
}


void Window::RecreateWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	pWindow = glfwCreateWindow(iWindowWidth, iWindowHeight, "VISSA", NULL, NULL);
	if (pWindow == NULL)
	{
		assert(!"Failed to create GFLW window\n");
		glfwTerminate();
	}
	glfwMakeContextCurrent(pWindow);
}

void Window::ProcessInputs()
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);
}
