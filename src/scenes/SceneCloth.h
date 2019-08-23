#pragma once

#include <iostream>
#include <vector>

#include "../Scene.h"
#include "../GLGUI.h"
#include "../Loader.h"
#include "../Plane.h"

#include "../shaderProgram.h"


class SceneCloth : public Scene {

public:
	SceneCloth();
	SceneCloth(int w, int h);

	void initScene();
	void update(float dt);
	void render();
	void resize(int w, int h);

private:
	int width, height;

	ShaderProgram prog, progCloth, progClothNorm;

	GLuint readBuf;
	GLuint posBufs[2], velBufs[2];
	GLuint normBuf, texcBuf, elBuf;

	float time, deltaT, speed;
	GLuint clothVAO;
	glm::vec2 nParticles;
	glm::vec2 clothSize;
	GLuint numElements;

	Plane plane;

	void initBuffers();
	void setMatrices();
	void compileAndLinkShaders();

	void renderGUI();
};