#pragma once

// This is a scene for SPH fluid simulation
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "../Scene.h"
#include "../Plane.h"
#include "../GLGUI.h"
#include "../GLUtils.h"

#include "../shaderProgram.h"


class SceneSPH : public Scene {

public:
	SceneSPH();
	SceneSPH(int w, int h);

	void initScene();
	

	void update(float dt);
	void render();
	void resize(int w, int h);

private:
	int width, height;

	ShaderProgram prog, progIntegrate, progPres, progForce;
	GLuint sphVAO;

	Plane plane;

	GLuint posBuf, velBuf, forceBuf, densityBuf, presBuf, normBuf;

	GLuint ParticleNum;
	glm::vec3 nParticles;
	glm::vec3 initSize;

	GLuint WORK_GROUP_SIZE;
	GLuint WORK_GROUP_NUM;

	void initBuffers();
	void setMatrices();
	void compileAndLinkShaders();

	void renderGUI();

};