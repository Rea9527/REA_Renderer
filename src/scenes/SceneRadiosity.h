#pragma once

#include <unordered_map>

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

#include "../GLGUI.h"
#include "../GLUtils.h"


class SceneRadiosity : public Scene {

public:
	SceneRadiosity();
	SceneRadiosity(int w, int h);

	void initScene();
	void update(float dt);
	void render();
	void resize(int w, int h);

private:
	ShaderProgram prog;
	unordered_map<string, ShaderProgram*> progList;

	// rendered objects
	Sphere m_sphere;
	Plane m_plane;

	// functions
	void compileAndLinkShaders();
	void setMatrices(string name);
	void renderGUI();
};