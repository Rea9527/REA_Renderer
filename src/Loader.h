#pragma once

#include <glad/include/glad/glad.h>
#include <string>

class Loader {

public:

	static GLuint loadTexture(const std::string & fName, GLint wrapMode);
	static GLuint loadCubeMap(const std::string & baseName);
	static unsigned char * loadPixels(const std::string & fName, int & w, int & h, int &bytesPerPix);
};