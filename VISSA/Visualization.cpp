#include "Visualization.h"

#include <assert.h>
#include "Engine.h"

Visualization::Visualization(Window* pMainWindow) :
	m_pMainWindow(pMainWindow),
	m_fDeltaTime(0.0f)
{
	assert(m_pMainWindow);
	assert(m_pMainWindow == Engine::GetMainWindow()); // just to protect from mistakes (duh). the ctor expects the main window to make the dependency clear
}

Visualization::~Visualization()
{
	//delete m_pMainWindow; the main window is only a borrowed resource. do not delete!
}

void Visualization::Load()
{
	assert(!"Visualization::Load called. Read below");
	/*
		The Visualization base class requires to default loading.
		Concrete Visualization implementations are required to handle their loading process themselves.
		Do not call this function.
	*/
}

void Visualization::Render()
{
	assert(!"Visualization::Render called. Read below.");
	/*
		the Visualization base class does no default rendering.
		Concrete Visualization implementations are responsible for all their rendering.
		Do not call this function.
	*/
}

void Visualization::Update(float fDeltaTime)
{
	assert(!"Visualization::Update base implementation called. Read below.");
	/*
		the Visualization Base class does not handle updates.
		Handling updates is up to the concrete Visualization implementation.
		Do not call this function.
	*/
}

void Visualization::MouseMoveCallback(GLFWwindow* pWindow, double dXPosition, double dYPosition)
{
	assert(!"Visualization::MouseMoveCallback base implementation called. Read below.");
	/*
		the Visualization Base class does not handle mouse movements.
		Handling mouse movement is up to the concrete Visualization implementation.
		Do not call this function.
	*/
}

void Visualization::MouseClickCallback(GLFWwindow * pWindow, int iButton, int iAction, int iModifiers)
{
	assert(!"Visualization::MouseClickCallback base implementation called. Read below.");
	/*
		the Visualization Base class does not handle mouse clicks.
		Handling mouse clicks is up to the concrete Visualization implementation.
		Do not call this function.
	*/
}

void Visualization::WindowResizeCallBack(GLFWwindow * pWindow, int iNewWidth, int iNewHeight)
{
	assert(!"Visualization::WindowResizeCallBack base implementation called. Read below.");
	/*
		the Visualization Base class does not handle window resizing.
		Handling mouse movement is up to the concrete Visualization implementation.
		Do not call this function.
	*/
}

void Visualization::ProcessKeyboardInput()
{
	assert(!"Visualization::ProcessKeyboardInput base implementation called. Read below.");
	/*
		the Visualization Base class does not process keyboard inputs.
		Processing keyboard inputs is up to the concrete Visualization implementation.
		Do not call this function.
	*/
}