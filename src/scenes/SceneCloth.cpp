
#include "SceneCloth.h"


#define PRIM_RESTART 0xffffff

SceneCloth::SceneCloth() : clothVAO(0), numElements(0),
							nParticles(40, 40), clothSize(4.0f, 3.0f),
							time(0.0f), deltaT(0.0f), speed(200.0f), readBuf(0), plane(10.0f, 10.0f, 100, 100) { }

SceneCloth::SceneCloth(int w, int h) : clothVAO(0), numElements(0),
									nParticles(40, 40), clothSize(4.0f, 3.0f),
									time(0.0f), deltaT(0.0f), speed(200.0f), readBuf(0), plane(10.0f, 10.0f, 100, 100), width(w), height(h) { }

void SceneCloth::initScene() {

	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(PRIM_RESTART);

	this->compileAndLinkShaders();
	this->initBuffers();

	glEnable(GL_DEPTH_TEST);

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0.85f, 6.32f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -40.f, -270.0f);
	camera->setSpeed(50.0f);

	// set lights
	this->prog.use();
	this->prog.setUniform("LightIntensity", glm::vec3(0.9f, 0.9f, 0.9f));
	this->prog.setUniform("LightPosition", glm::vec4(0.8f, 0.8f, 0.4f, 1.0f));

	this->prog.setUniform("Kd", 0.7f, 0.7f, 0.7f);
	this->prog.setUniform("Ka", 0.2f, 0.2f, 0.2f);
	this->prog.setUniform("Ks", 0.2f, 0.2f, 0.2f);
	this->prog.setUniform("Shininess", 180.0f);

	// set cloth compute shader uniform
	this->progCloth.use();
	float dx = clothSize.x / (nParticles.x - 1);
	float dy = clothSize.y / (nParticles.y - 1);
	this->progCloth.setUniform("RestLenHoriz", dx);
	this->progCloth.setUniform("RestLenVert", dy);
	this->progCloth.setUniform("RestLenDiag", sqrtf(dx * dx + dy * dy));

	glActiveTexture(GL_TEXTURE0);
	Loader::loadTexture("./medias/textures/me_textile.png", GL_CLAMP_TO_EDGE);
}

void SceneCloth::initBuffers() {

	// init particle poitions and velocity
	// bind data to the buffer
	glm::mat4 transf = glm::translate(glm::mat4(1.0), glm::vec3(0, clothSize.y, 0));
	transf = glm::rotate(transf, glm::radians(-80.0f), glm::vec3(1, 0, 0));
	transf = glm::translate(transf, glm::vec3(0, -clothSize.y, 0));

	vector<GLfloat> initPos, initTexc;
	// init velocity to 0.0f
	vector<GLfloat> initVel(nParticles.x * nParticles.y * 4, 0.0f);

	float deltaX = this->clothSize.x / (this->nParticles.x - 1);
	float deltaY = this->clothSize.y / (this->nParticles.y - 1);
	float deltaS = 1.0f / (this->nParticles.x - 1);
	float deltaT = 1.0f / (this->nParticles.y - 1);

	glm::vec4 p(0.0f, 0.0f, 0.0f, 1.0f);
	// init particle position and texcoord
	for (int i = 0; i < nParticles.y; i++) {
		for (int j = 0; j < nParticles.x; j++) {
			p.x = deltaX * j;
			p.y = deltaY * i;
			p.z = 0.0f;
			p = transf * p;
			initPos.push_back(p.x);
			initPos.push_back(p.y);
			initPos.push_back(p.z);
			initPos.push_back(1.0f);

			initTexc.push_back(j * deltaS);
			initTexc.push_back(i * deltaT);
		}
	}

	// each adjacent two rows form one triangle strip
	vector<GLuint> el;
	for (int row = 0; row < nParticles.y - 1; row++) {
		for (int col = 0; col < nParticles.x; col++) {
			el.push_back((row + 1) * nParticles.x + col);
			el.push_back(row * nParticles.x + col);
		}
		el.push_back(PRIM_RESTART);
	}
	this->numElements = el.size();

	// bind buffers
	// double buffers for position and velocity, one buffer for element array, norm and texcood
	GLuint bufs[7];
	glGenBuffers(7, bufs);
	this->posBufs[0] = bufs[0];
	this->posBufs[1] = bufs[1];
	this->velBufs[0] = bufs[2];
	this->velBufs[1] = bufs[3];
	this->normBuf = bufs[4];
	this->texcBuf = bufs[5];
	this->elBuf = bufs[6];

	GLuint particles_num = nParticles.x * nParticles.y;
	// position buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->posBufs[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particles_num * 4 * sizeof(GLfloat), &initPos[0], GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->posBufs[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particles_num * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	// velocity buffers
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->velBufs[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particles_num * 4 * sizeof(GLfloat), &initVel[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, this->velBufs[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particles_num * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	// normal buffer
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, normBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, particles_num * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);

	// texcoord buffer
	glBindBuffer(GL_ARRAY_BUFFER, texcBuf);
	glBufferData(GL_ARRAY_BUFFER, initTexc.size() * sizeof(GLfloat), &initTexc[0], GL_STATIC_DRAW);

	// element buffer
	glBindBuffer(GL_ARRAY_BUFFER, elBuf);
	glBufferData(GL_ARRAY_BUFFER, el.size() * sizeof(GLuint), &el[0], GL_DYNAMIC_COPY);


	// generate VAO
	glGenVertexArrays(1, &this->clothVAO);
	glBindVertexArray(this->clothVAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->posBufs[0]);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, this->normBuf);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, this->texcBuf);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elBuf);

	glBindVertexArray(0);

}

void SceneCloth::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), this->width / (float)this->height, 1.0f, 1000.0f);

	//printf("%f, %f, %f, %f, %f", camera.getPos().x, camera.getPos().y, camera.getPos().z, camera.getPitch(), camera.getYaw());

	if (this->time == 0.0f) {
		this->deltaT = 0.0f;
	}
	else {
		this->deltaT = dt - this->time;
	}
	this->time = dt;
}

void SceneCloth::render() {

	// cloth program
	this->progCloth.use();
	for (int i = 0; i < 1000; i++) {
		glDispatchCompute(this->nParticles.x / 10, this->nParticles.y / 10, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// swap buffer for communication
		this->readBuf = 1 - this->readBuf;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->posBufs[this->readBuf]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->posBufs[1 - this->readBuf]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->posBufs[this->readBuf]);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, this->posBufs[1 - this->readBuf]);

	}

	// cloth normal program
	this->progClothNorm.use();
	glDispatchCompute(this->nParticles.x / 10, this->nParticles.y / 10, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// rendering program
	// draw the scene
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->renderGUI();

	this->prog.use();

	this->model = glm::mat4(1.0f);
	this->model = glm::translate(model, glm::vec3(0.0f, -0.9f, 0.0f));
	this->setMatrices();
	this->plane.render();

	this->model = glm::mat4(1.0f);
	this->model = glm::translate(this->model, glm::vec3(0.0f, this->clothSize.y, 0.0f));
	this->setMatrices();
	// draw the cloth
	glBindVertexArray(clothVAO);
	glDrawElements(GL_TRIANGLE_STRIP, this->numElements, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);


}

void SceneCloth::resize(int w, int h) {
	glViewport(0, 0, w, h);
	this->width = w;
	this->height = h;
}

void SceneCloth::setMatrices() {

	this->prog.use();
	glm::mat4 mv = this->view * this->model;
	glm::mat3 norm = glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]));

	this->prog.setUniform("ModelViewMatrix", mv);
	this->prog.setUniform("NormalMatrix", norm);
	this->prog.setUniform("MVP", this->projection * mv);
}

void SceneCloth::compileAndLinkShaders() {

	try {
		this->prog.compileShader("./medias/SceneCloth/ads.vert", GLSLShader::VERTEX);
		this->prog.compileShader("./medias/SceneCloth/ads.frag", GLSLShader::FRAGMENT);
		this->prog.link();

		this->progCloth.compileShader("./medias/SceneCloth/cloth.comp", GLSLShader::COMPUTE);
		this->progCloth.link();

		this->progClothNorm.compileShader("./medias/SceneCloth/clothNorm.comp", GLSLShader::COMPUTE);
		this->progClothNorm.link();
	}
	catch (ShaderProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}

}


void SceneCloth::renderGUI() {
	// add a new frame
	ImGui_ImplGlfwGL3_NewFrame("Menu");

	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("Subdivision")) {
			if (ImGui::MenuItem("Enable")) {}
			if (ImGui::MenuItem("Disable")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//render the frame
	ImGui::Render();
}