
#include <iostream>
using namespace std;

#include "SceneToon.h"


SceneToon::SceneToon() : plane(100.0f, 100.0f, 10, 10), teapot(14, glm::mat4(1.0)), prog("adsShader"), progToon("toonShader"),
								renderFBO(800, 600, true, true) { }

SceneToon::SceneToon(int w, int h) : plane(100.0f, 100.0f, 10, 10), teapot(14, glm::mat4(1.0)), prog("adsShader"), progToon("toonShader"),
renderFBO(w, h, true, true), width(w), height(h) { }

void SceneToon::initScene() {

	// compile and link the shader
	this->compileAndLinkShaders();

	glEnable(GL_DEPTH_TEST);

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0.0f, 20.0f, 30.0f), glm::vec3(0.0f, 1.0f, 0.0f), -80.f, -20.0f);
	//this->view = camera.getViewMat();
	//this->projection = glm::perspective(glm::radians(camera.getZoom()), this->width / (float)this->height, 0.1f, 1000.0f);

	// generate a quad for the filter pass
	this->quadVAO = BufferObject::genQuadBufferObject();


	// set up uniforms for the plane shader
	prog.use();
	prog.setUniform("LightIntensity", glm::vec3(0.9f, 0.9f, 0.9f));
	prog.setUniform("LightPosition", glm::vec4(0.8f, 0.8f, 0.4f, 1.0f));
	prog.setUniform("Kd", 0.7f, 0.7f, 0.7f);
	prog.setUniform("Ka", 0.2f, 0.2f, 0.2f);
	prog.setUniform("Ks", 0.2f, 0.2f, 0.2f);
	prog.setUniform("Shininess", 180.0f);

	// set up uniforms for the toon shader
	progToon.use();
	progToon.setUniform("LightIntensity", glm::vec3(0.9f, 0.9f, 0.9f));
	progToon.setUniform("LightPosition", glm::vec4(0.8f, 0.8f, 0.4f, 1.0f));
	progToon.setUniform("Kd", 0.7f, 0.7f, 0.7f);
	progToon.setUniform("Ka", 0.2f, 0.2f, 0.2f);
	progToon.setUniform("edgeThreshold", 0.05f);
	
	GLuint toonhandle = progToon.getHandle();
	this->filterPassInx = glGetSubroutineIndex(toonhandle, GL_FRAGMENT_SHADER, "filtering");
	this->shadingPassInx = glGetSubroutineIndex(toonhandle, GL_FRAGMENT_SHADER, "shading");

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}


void SceneToon::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), this->width / (float)this->height, 0.1f, 1000.0f);

	//printf("%f, %f\n", camera.getYaw(), camera.getPitch());
}

void SceneToon::render() {
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->renderGUI();

	// plane shader
	this->prog.use();

	this->model = glm::mat4(1.0f);
	this->model = glm::translate(model, glm::vec3(0.0f, -0.9f, 0.0f));
	this->setMatrices(this->prog.getName());
	this->plane.render();

	//toon shader
	this->progToon.use();

	this->shadingPass();
	glFlush();
	this->filterPass();

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneToon::shadingPass() {
	this->renderFBO.bind();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->shadingPassInx);

	this->model = glm::mat4(1.0f);
	this->model = glm::translate(model, glm::vec3(0.0, 1.0f, 0.0f));
	this->setMatrices(this->progToon.getName());
	this->teapot.render();

	// GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneToon::filterPass() {
	this->renderFBO.unbind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->renderFBO.getRTTHandle());

	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->filterPassInx);
	this->model = glm::mat4(1.0f);
	this->view = glm::mat4(1.0f);
	this->projection = glm::mat4(1.0f);
	this->setMatrices(this->progToon.getName());

	//this->teapot.render();

	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	//GLUtils::checkForOpenGLError(__FILE__, __LINE__);

}


void SceneToon::renderGUI() {
	// add a new frame
	ImGui_ImplGlfwGL3_NewFrame("Menu");

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Enable")) {}
			if (ImGui::MenuItem("Disable")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//render the frame
	ImGui::Render();
}

void SceneToon::setMatrices(string pname) {
	ShaderProgram* program = this->programs[pname];
	glm::mat4 mv = this->view * this->model;
	program->setUniform("ModelViewMatrix", mv);
	program->setUniform("NormalMatrix",
		glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	program->setUniform("MVP", this->projection * mv);
}

void SceneToon::resize(int w, int h) {
	glViewport(0, 0, w, h);
	this->width = w;
	this->height = h;
	this->projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneToon::compileAndLinkShaders() {
	try {
		prog.compileShader("./medias/adsShader.vert", GLSLShader::VERTEX);
		prog.compileShader("./medias/adsShader.frag", GLSLShader::FRAGMENT);
		prog.link();
		this->programs.insert(std::pair<string, ShaderProgram*> (prog.getName(), &prog));

		progToon.compileShader("./medias/toonShader.vert", GLSLShader::VERTEX);
		progToon.compileShader("./medias/toonShader.frag", GLSLShader::FRAGMENT);
		progToon.link();
		this->programs.insert(std::pair<string, ShaderProgram*>(progToon.getName(), &progToon));
	}
	catch (ShaderProgramException & e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}
