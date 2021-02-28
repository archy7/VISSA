#include "generalGL.h"

#include "Window.h"
#include <assert.h>

int main(int argc, char** argv)
{
	Window::InitializationSequence();

	while (!glfwWindowShouldClose(Window::pWindow))
	{
		Window::ProcessInputs();

		glfwSwapBuffers(Window::pWindow);
		glfwPollEvents();
	}

	Window::CleanUpSequence();
	return 0;
}