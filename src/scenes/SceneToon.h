#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <map>

#include "../shaderProgram.h"
#include "../FrameBuffer.h"
#include "../BufferObject.h"

#include "../Scene.h"
#include "../Plane.h"
#include "../teapot.h"
#include "../GLGUI.h"

#include "../GLUtils.h"

class SceneToon : public Scene {

public:
	SceneToon();
	SceneToon(int w, int h);

	void initScene();
	void update(float t);
	void render();
	void resize(int w, int h);


private:
	int width, height;

	map<string, ShaderProgram*> programs;
	ShaderProgram prog, progToon;

	FrameBuffer renderFBO;
	// quad for filterPass
	GLuint quadVAO;

	Plane plane;
	Teapot teapot;

	void filterPass();
	void shadingPass();

	GLuint filterPassInx;
	GLuint shadingPassInx;

	void setMatrices(string pname);
	void compileAndLinkShaders();

	void renderGUI();
};