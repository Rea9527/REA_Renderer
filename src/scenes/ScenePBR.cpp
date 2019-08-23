#include "ScenePBR.h"


ScenePBR::ScenePBR() :m_sphere(2.0f, 50, 50),
					   m_plane(200, 100, 1, 1),
					   progPBR("pbrShader") { }

ScenePBR::ScenePBR(int w, int h) : Scene(w, h),
								   m_sphere(2.0f, 50, 50),
								   m_plane(200, 100, 1, 1),
								   progPBR("pbrShader") { }


void ScenePBR::initScene() {
	this->compileAndLinkShaders();

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0, 0.0, 30.0), glm::vec3(0, 1.0, 0), -90, 0);

	// set uniforms
	this->progPBR.use();
	
	this->progPBR.setUniform("Albedo", vec3(0.5f, 0.0f, 0.0f));
	this->progPBR.setUniform("AOFactor", 1.0f);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ScenePBR::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), (float)this->width / (float)this->height, 0.1f, 100.0f);
}

void ScenePBR::resize(int w, int h) {
	this->width = w;
	this->height = h;
	glViewport(0, 0, w, h);
	// should adjust according to camera 
	this->projection = glm::perspective(glm::radians(Camera::getInstance()->getZoom()), (float)w / h, 0.3f, 100.0f);
}

void ScenePBR::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	this->progPBR.use();

	GLint offset1[] = { -1, 1, -1, -1, 1, 1, 1, -1 };
	for (int i = 0; i < 4; i++) {
		string str = "Light[" + to_string(i) + "].Color";
		this->progPBR.setUniform(str.c_str(), vec3(400.0f, 400.0f, 400.0f));
		str = "Light[" + to_string(i) + "].Position";
		vec3 worldpos = vec3(offset1[i * 2] * 10.0f, offset1[i * 2 + 1] * 10.0f, 15.0f);
		this->progPBR.setUniform(str.c_str(), 
			vec3(this->view * vec4(worldpos, 1.0f)));

		this->model = glm::translate(mat4(1.0f), worldpos);
		this->setMatrices(this->progPBR.getName());
		this->m_sphere.render();
	}

	// camera position
	this->progPBR.setUniform("CamPos", Camera::getInstance()->getPos());

	GLfloat metallic[5] = {0.0, 0.25, 0.5, 0.75, 1.0};
	GLfloat roughness[5] = {0.0, 0.25, 0.5, 0.75, 1.0};
	GLint off[5] = { -2, -1, 0, 1, 2 };
	// render 25 spheres
	for (int i = 0; i < 5; i++) {
		this->progPBR.setUniform("Metallic", metallic[i]);
		for (int j = 0; j < 5; j++) {
			this->progPBR.setUniform("Roughness", roughness[j]);
			vec3 pos = vec3(off[i] * 10.0f, off[j] * 10.0f, 0.0f);
			this->model = glm::translate(mat4(1.0f), pos);
			this->setMatrices(this->progPBR.getName());
			this->m_sphere.render();
			
		}
	}


	this->renderGUI();

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}


void ScenePBR::setMatrices(string name) {
	ShaderProgram* prog = progList[name];
	mat4 mv = this->view * this->model;
	prog->setUniform("ModelViewMatrix", mv);
	mat4 nm = glm::transpose(glm::inverse(mv));
	prog->setUniform("NormalMatrix",
		mat3(vec3(nm[0]), vec3(nm[1]), vec3(nm[2])));
	prog->setUniform("MVP", this->projection * mv);

}

void ScenePBR::compileAndLinkShaders() {
	this->progPBR.compileShader("./medias/pbrShader.vert", GLSLShader::VERTEX);
	this->progPBR.compileShader("./medias/pbrShader.frag", GLSLShader::FRAGMENT);
	this->progPBR.link();
	this->progList.insert({ progPBR.getName(), &this->progPBR });

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ScenePBR::renderGUI() {

}