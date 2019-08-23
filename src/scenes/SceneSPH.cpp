
#include "SceneSPH.h"


SceneSPH::SceneSPH() : nParticles(glm::vec3(16, 16, 16)), initSize(glm::vec3(1.0f, 1.0f, 1.0f)), sphVAO(0), WORK_GROUP_SIZE(64), plane(10.0f, 10.0f, 100, 100) {

	this->ParticleNum = nParticles.x * nParticles.y * nParticles.z;
	this->WORK_GROUP_NUM = (this->ParticleNum + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
}

SceneSPH::SceneSPH(int w, int h) : nParticles(glm::vec3(16, 16, 16)), initSize(glm::vec3(1.0f, 1.0f, 1.0f)), 
									sphVAO(0), WORK_GROUP_SIZE(64), plane(10.0f, 10.0f, 100, 100),
									width(w), height(h) {

	this->ParticleNum = nParticles.x * nParticles.y * nParticles.z;
	this->WORK_GROUP_NUM = (this->ParticleNum + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
}

void SceneSPH::initScene() {

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

	this->compileAndLinkShaders();
	this->initBuffers();

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0.0, 5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), 60.0f, 0.0f);
	camera->setSpeed(50);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
	// bind light uniforms
	this->prog.use();
	this->prog.setUniform("LightIntensity", glm::vec3(0.9f, 0.9f, 0.9f));
	this->prog.setUniform("LightPosition", glm::vec4(0.8f, 0.8f, 0.5f, 1.0f));
	this->prog.setUniform("Ka", glm::vec3(0.7f, 0.7f, 0.7f));
	this->prog.setUniform("Kd", glm::vec3(0.7f, 0.7f, 0.7f));
	this->prog.setUniform("Ks", glm::vec3(0.7f, 0.7f, 0.7f));

	// bind pressure uniforms
	this->progPres.use();
	this->progPres.setUniform("nParticles", this->ParticleNum);

	// bind force uniforms
	this->progForce.use();
	this->progForce.setUniform("nParticles", this->ParticleNum);

	// bind integration uniforms
	this->progIntegrate.use();
	this->progIntegrate.setUniform("nParticles", this->ParticleNum);

	// bind texture

}

void SceneSPH::initBuffers() {

	vector<GLfloat> initPos;
	vector<GLfloat> initVel(this->ParticleNum * 4, 0.0f), initForce(this->ParticleNum * 4, 0.0f),
					initDensity(this->ParticleNum, 0.0f), initPres(this->ParticleNum, 0.0f);

	float deltaX = initSize.x / (nParticles.x - 1);
	float deltaY = initSize.y / (nParticles.y - 1);
	float deltaZ = initSize.z / (nParticles.z - 1);

	glm::vec4 p = glm::vec4(0.0, 0.0, 0.0, 1.0);
	for (int x = 0; x < nParticles.x; x++) {
		for (int y = 0; y < nParticles.y; y++) {
			for (int z = 0; z < nParticles.z; z++) {
				p.x = x * deltaX;
				p.y = y * deltaY;
				p.z = z * deltaZ;
				initPos.push_back(p.x);
				initPos.push_back(p.y);
				initPos.push_back(p.z);
				initPos.push_back(1.0f);
			}
		}
	}

	GLuint bufs[6];
	glGenBuffers(6, bufs);
	this->posBuf = bufs[0];
	this->velBuf = bufs[1];
	this->forceBuf = bufs[2];
	this->densityBuf = bufs[3];
	this->presBuf = bufs[4];
	this->normBuf = bufs[5];

	// bind position
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->posBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * 4 * sizeof(GLfloat), &initPos[0], GL_DYNAMIC_DRAW);

	// bind vel
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->velBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * 4 * sizeof(GLfloat), &initVel[0], GL_DYNAMIC_COPY);

	// bind force
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->forceBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	// bind density
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, this->densityBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	// bind pressure
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, this->presBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	// bind normal
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, this->normBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->ParticleNum * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);


	// bind VAO
	glGenVertexArrays(1, &this->sphVAO);
	glBindVertexArray(this->sphVAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->posBuf);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, this->normBuf);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);

}

void SceneSPH::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), this->width / (float)this->height, 0.1f, 1000.0f);

}

void SceneSPH::render() {

	this->progPres.use();
	glDispatchCompute(this->WORK_GROUP_NUM, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	this->progForce.use();
	glDispatchCompute(this->WORK_GROUP_NUM, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	this->progIntegrate.use();
	glDispatchCompute(this->WORK_GROUP_NUM, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// render scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->renderGUI();

	this->prog.use();

	this->model = glm::mat4(1.0f);
	this->setMatrices();
	this->plane.render();

	this->model = glm::mat4(1.0f);
	this->setMatrices();
	glBindVertexArray(this->sphVAO);
	glDrawArrays(GL_POINTS, 0, this->ParticleNum);
	glBindVertexArray(0);
}

void SceneSPH::resize(int w, int h) {
	glViewport(0, 0, w, h);
	this->width = w;
	this->height = h;
}


void SceneSPH::setMatrices() {

	glm::mat4 mv = this->view * this->model;
	glm::mat3 normMat = glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]));

	this->prog.use();
	this->prog.setUniform("ModelViewMatrix", mv);
	this->prog.setUniform("NormalMatrix", normMat);
	this->prog.setUniform("MVP", this->projection * mv);
}
	
void SceneSPH::compileAndLinkShaders() {

	try {
		this->prog.compileShader("./medias/SceneSPH/ads.vert", GLSLShader::VERTEX);
		this->prog.compileShader("./medias/SceneSPH/ads.frag", GLSLShader::FRAGMENT);
		this->prog.link();

		this->progIntegrate.compileShader("./medias/SceneSPH/sphIntegration.comp", GLSLShader::COMPUTE);
		this->progIntegrate.link();

		this->progForce.compileShader("./medias/SceneSPH/sphForce.comp", GLSLShader::COMPUTE);
		this->progForce.link();

		this->progPres.compileShader("./medias/SceneSPH/sphPressure.comp", GLSLShader::COMPUTE);
		this->progPres.link();
	}
	catch (ShaderProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}

}

	
void SceneSPH::renderGUI() {

}