#pragma once

#include "generalGL.h"

namespace Window {
	extern GLFWwindow* pWindow;
	extern int iWindowWidth;
	extern int iWindowHeight;
	
	void InitializationSequence();
	void ReInitializationSequence();
	void CleanUpSequence();
	void RecreateWindow();

	void ProcessInputs();

	// Callbacks
	void ResizeWindowCallback(GLFWwindow* window, int width, int height);
};