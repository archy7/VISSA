#include "Window.h"

#include <assert.h>
#include <stdio.h>
#include "Engine.h"

Window::Window():
	m_pGLFWwindow(nullptr),
	m_iWindowWidth(1280),
	m_iWindowHeight(720),
	m_fLastXOfMouse(m_iWindowWidth/ 2.0f),
	m_fLastYOfMouse(m_iWindowHeight / 2.0f),
	m_bFirstMouse(true),
	m_bMouseCaptured(false),
	m_bIsInitialized(false)
{
	
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

	glViewport(0, 0, m_iWindowWidth, m_iWindowHeight);
	glfwSetFramebufferSizeCallback(m_pGLFWwindow, Engine::ResizeWindowCallback);
	glfwSetCursorPosCallback(m_pGLFWwindow, Engine::MouseMoveCallBack);
	glfwSetScrollCallback(m_pGLFWwindow, Engine::MouseScrollCallBack);

	SetMouseCaptured(m_bMouseCaptured);	

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



void Window::RecreateWindow()
{
	assert(m_iWindowWidth > 0);
	assert(m_iWindowHeight > 0);

	glfwSetErrorCallback(Engine::GLFWErrorCallBack);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
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

void Window::SetMouseCaptured(bool bIsCaptured)
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