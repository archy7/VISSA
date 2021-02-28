#include "generalGL.h"


#include <stdio.h>	// printf
#include <assert.h>

// To force dedicated nvidia GPUs to run
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

// To force dedicated AMD GPUs to run
extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

void glAssertAllIsFine(const char* sFile, int iLine)
{
#ifdef _DEBUG

	GLenum uiErrorCode = glGetError();

	switch (uiErrorCode)
	{
	case GL_NO_ERROR: {/* do nothing*/break; };
	case GL_INVALID_ENUM: printf("GL_ERROR:: GL_INVALID_ENUM "); break;
	case GL_INVALID_VALUE: printf("GL_ERROR:: GL_INVALID_VALUE "); break;
	case GL_INVALID_OPERATION: printf("GL_ERROR:: GL_INVALID_OPERATION "); break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: printf("GL_ERROR:: GL_INVALID_FRAMEBUFFER_OPERATION "); break;
	case GL_OUT_OF_MEMORY: printf("GL_ERROR:: GL_OUT_OF_MEMORY "); break;
	default: assert(!"ERROR: DEFAULT CASE HIT IN glCheckError!");
	}

	if (uiErrorCode != GL_NO_ERROR)
	{
		printf("at %d in %s\n", iLine, sFile);
	}

#endif // _DEBUG
}