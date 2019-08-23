#pragma once

#include "../SceneManager.h"
#include "../Scene.h"
#include "../Camera.h"
#include "../shaderProgram.h"
#include "../BufferObject.h"
#include "../Loader.h"

#include "../teapot.h"
#include "../Sphere.h"
#include "../Plane.h"
#include "../Model.h"
#include "../SkyBox.h"

#include "../GLGUI.h"
#include "../GLUtils.h"




class SceneSSAO : public Scene {

public:
	SceneSSAO();
	SceneSSAO(int w, int h);

	void initScene();
	void update(float dt);
	void render();
	void resize(int w, int h);

private:
	map<string, ShaderProgram*> progList;
	ShaderProgram prog, progIns, progSkybox;

	Teapot m_teapot;
	GLuint m_teapot_count;
	Sphere m_sphere;
	Plane m_plane;
	SkyBox m_skybox;
	// simple Quad for the second pass
	GLuint quadVAO;

	// two passed for defer shading
	GLuint geometryPassInx, ssaoPassInx, ssaoBlurPassInx, lightingPassInx;
	// gbuffer
	GLuint gBuffer;
	// FBO
	GLuint ssaoFBO, ssaoBlurFBO;
	// texture ids
	GLuint gPos, gNorm, gColor;
	GLuint ssaoBuf, ssaoBlurBuf;
	GLuint noiseTex;

	// func
	void computeSSAOKernelsAndNoises();
	void setupGBuffer();
	void setupFBO();
	void createGBufferTex(GLenum format, GLuint &texId, bool wrap = false);
	float lerp(float a, float b, float val);

	void geometryPass();
	void ssaoPass();
	void ssaoBlurPass();
	void lightingPass();

	void compileAndLinkShaders();
	void setMatrices(string name);
	void renderGUI();
	void drawScene();
};