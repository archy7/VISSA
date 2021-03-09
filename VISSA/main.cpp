#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "generalGL.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"
#include "GUI.h"

int main()
{
	Window tMainWindow;
	tMainWindow.InitWindow();

	Renderer tRenderer;
	tRenderer.InitRenderer();
	
	Camera tCamera(glm::vec3(0.0f, 100.0f, 400.0f));
	// this is part of a quick and dirty solution to find the non-global camera in callback functions with a fixed signature. There are better and more sophisticated ways to do this.
	Window::GLFWwindowLookUpTuple.m_pAssociatedCamera = &tCamera;

	GUI tGUI;
	tGUI.InitForWindow(tMainWindow);
	// this is part of a quick and dirty solution to find the non-global camera in callback functions with a fixed signature. There are better and more sophisticated ways to do this.
	Window::GLFWwindowLookUpTuple.m_pAssociatedGUI = &tGUI;

	// timing
	float fDeltaTime = 0.0f;
	float fLastFrame = 0.0f;

	glAssert();

	// render loop
	// -----------
	while (!tMainWindow.WindowShouldClose())
	{
		// per-frame time logic
		// --------------------
		float fCurrentFrame = static_cast<float>(glfwGetTime()); // float is good enough 
		fDeltaTime = fCurrentFrame - fLastFrame;
		fLastFrame = fCurrentFrame;

		// input
		// -----
		glfwPollEvents();	// ask GLFW what kind of input happened
		Window::ProcessKeyboardInput(tMainWindow.m_pGLFWwindow, fDeltaTime); // process inputs accordingly

		// render
		tRenderer.RenderFrame(tCamera, tMainWindow, tGUI);

		// glfw: swap buffers
		glfwSwapBuffers(tMainWindow.m_pGLFWwindow);
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}