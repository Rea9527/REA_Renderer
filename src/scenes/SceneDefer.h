#pragma once


#include "../Scene.h"
#include "../Camera.h"
#include "../shaderProgram.h"
#include "../BufferObject.h"
#include "../Loader.h"

#include "../teapot.h"
#include "../Sphere.h"
#include "../Plane.h"
#include "../Model.h"

#include "../GLGUI.h"
#include "../GLUtils.h"




class SceneDefer : public Scene {

public:
	SceneDefer();
	SceneDefer(int w, int h);

	void initScene();
	void update(float dt);
	void render();
	void resize(int w, int h);

private:
	map<string, ShaderProgram*> progList;
	ShaderProgram prog, progIns;

	Teapot m_teapot;
	GLuint m_teapot_count;
	Sphere m_sphere;
	Plane m_plane;
	// simple Quad for the second pass
	GLuint quadVAO;

	// two passed for defer shading
	GLuint geometryPassInx, lightingPassInx;
	// gbuffer
	GLuint gBuffer;
	// texture ids
	GLuint gPos, gNorm, gColor;

	// func
	void setupGBuffer();
	void createGBufferTex(GLenum format, GLuint &texId);

	void compileAndLinkShaders();
	void setMatrices(string name);
	void renderGUI();
	void drawScene();
};