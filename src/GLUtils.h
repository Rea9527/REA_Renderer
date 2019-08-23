#pragma once

#include <glad/include/glad/glad.h>
#include <cstdio>
#include <iostream>

namespace GLUtils {
	int checkForOpenGLError(const char*, int);

	void dumpGLInfo(bool dumpExtensions = false);
}