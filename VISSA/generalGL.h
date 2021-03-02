#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdint.h>

void glAssertAllIsFine(const char* sFile, int iLine);

#define glAssert() glAssertAllIsFine(__FILE__, __LINE__)