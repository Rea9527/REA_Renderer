#pragma once

#include <glad/include/glad/glad.h>

#include <string>

class Texture {
public:
	Texture();
	Texture(GLuint id, std::string type, std::string path);


	GLuint id;
	std::string type;
	std::string path;
};