#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "generalGL.h"
#include "Camera.h"
#include "Window.h"
#include "Renderer.h"

int main()
{
	Window tMainWindow;
	tMainWindow.InitWindow();

	Renderer tRenderer;
	tRenderer.InitRenderer();
	
	Camera tCamera(glm::vec3(0.0f, 0.0f, 300.0f));
	// this is part of a quick and dirty solution to find the non-global camera in callback functions with a fixed signature. There are better and more sophisticated ways to do this.
	Window::GLFWwindowLookUpTable[0].m_pAssociatedCamera = &tCamera;

	// timing
	float fDeltaTime = 0.0f;	// time between current frame and last frame
	float fLastFrame = 0.0f;

	glAssert();

	// render loop
	// -----------
	while (!tMainWindow.WindowShouldClose())
	{
		// per-frame time logic
		// --------------------
		float fCurrentFrame = glfwGetTime();
		fDeltaTime = fCurrentFrame - fLastFrame;
		fLastFrame = fCurrentFrame;

		// input
		// -----
		Window::ProcessInput(tMainWindow.m_pGLFWwindow, fDeltaTime);

		// render
		tRenderer.RenderFrame(tCamera, tMainWindow);		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(tMainWindow.m_pGLFWwindow);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}