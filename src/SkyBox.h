#pragma once

#include "Drawable.h"
#include <glad/include/glad/glad.h>


class SkyBox : public Drawable {

public:
	SkyBox();

	void render() const;

	void setCubeMapId(GLuint id);

private:
	GLuint vao;
	GLuint cubeMapId;
};