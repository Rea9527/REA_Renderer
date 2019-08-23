#include "SceneRadiosity.h"


SceneRadiosity::SceneRadiosity() : m_sphere(2.0f, 50, 50),
					               m_plane(200, 100, 1, 1),
					               prog("radiosityShader") { }

SceneRadiosity::SceneRadiosity(int w, int h) : Scene(w, h),
                                               m_sphere(2.0f, 50, 50),
					                           m_plane(200, 100, 1, 1),
					                           prog("radiosityShader") { }


void SceneRadiosity::initScene() {
    this->compileAndLinkShaders();

    Camera::getInstance()->init(glm::vec3(0, 0.0, 30.0));

    
}


void SceneRadiosity::update(float dt) {

}

void SceneRadiosity::render() {

}

void SceneRadiosity::resize(int w, int h) {

}

void SceneRadiosity::compileAndLinkShaders() {

}

void SceneRadiosity::setMatrices(string name) {

}

void SceneRadiosity::renderGUI() {

}
