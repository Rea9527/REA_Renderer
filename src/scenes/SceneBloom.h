#pragma once



#include "../Scene.h"
#include "../Camera.h"
#include "../shaderProgram.h"
#include "../BufferObject.h"
#include "../Loader.h"
#include "../SkyBox.h"

#include "../teapot.h"
#include "../Sphere.h"
#include "../Plane.h"

#include "../GLGUI.h"
#include "../GLUtils.h"


class SceneBloom : public Scene {

public:
	SceneBloom();
	SceneBloom(int w, int h);
	~SceneBloom();

	void initScene();
	void update(float t);
	void render();
	void resize(int w, int h);


private:
	GLuint width, height;

	GLuint bloomBufWid, bloomBufHei;

	map<string, ShaderProgram*> programsList;
	ShaderProgram progSkybox, progBloom;

	Plane plane;
	Teapot teapot;
	Sphere sphere;
	SkyBox skybox;

	// frame buffer
	GLuint hdrFBO, blurFBO;
	GLuint rttHdr, rttBright, rttBlur;

	//sampler
	GLuint linearSampler, nearestSampler;

	//FrameBuffer hdrFBO, blurFBO;
	GLuint quadVAO;

	//cube map id
	GLuint cubeMapId;

	// five pass
	GLuint renderPassInx, brightPassInx, verGaussPassInx, horGaussPassInx, tonePassInx;
	void renderPass();
	void brightPass();
	void verGaussPass();
	void horGaussPass();
	void tonePass();

	// draw the scene in the render pass
	void setupFBO();
	void drawScene();
	void computeLogAvgLuminance();
	float gauss(int x, float sigma2);

	void setMatrices(string pname);
	void compileAndLinkShaders();
	void renderGUI();
};