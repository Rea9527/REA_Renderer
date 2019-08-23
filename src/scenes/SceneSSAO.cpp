#include "SceneSSAO.h"
#include <random>


SceneSSAO::SceneSSAO() : m_teapot(20, mat4(1.0f)), m_teapot_count(100),
						m_sphere(2.0f, 50, 50), m_plane(200, 100, 1, 1),
						prog("ssaoShader"), progIns("instancingShader"), progSkybox("skyboxShader") { }

SceneSSAO::SceneSSAO(int w, int h) : Scene(w, h),
									m_teapot(20, mat4(1.0f)), m_teapot_count(100),
									m_sphere(2.0f, 50, 50), m_plane(200, 100, 1, 1),
									prog("ssaoShader"), progIns("instancingShader"), progSkybox("skyboxShader") { }


void SceneSSAO::initScene() {
	this->compileAndLinkShaders();

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0, 0.0, 30.0), glm::vec3(0, 1.0, 0), -90, 0);

	// a simple quad for the second pass rendering
	this->quadVAO = BufferObject::genQuadBufferObject();

	this->prog.use();
	GLuint handle = this->prog.getHandle();
	this->geometryPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "geometryPass");
	this->ssaoPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "ssaoPass");
	this->ssaoBlurPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "ssaoBlurPass");
	this->lightingPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "lightingPass");
	this->prog.setUniform("Light.Color", vec3(0.9f, 0.9f, 0.9f));
	this->prog.setUniform("Light.Constant", 1.0f);
	this->prog.setUniform("Light.Linear", 0.007f);
	this->prog.setUniform("Light.Quadratic", 0.0002f);
	this->prog.setUniform("uRadius", 0.5f);
	// compute SSAO kernel and random rotation noises
	this->computeSSAOKernelsAndNoises();

	this->progIns.use();
	this->progIns.setUniform("Light.Color", vec3(0.9f, 0.9f, 0.9f));
	this->progIns.setUniform("Light.Constant", 1.0f);
	this->progIns.setUniform("Light.Linear", 0.007f);
	this->progIns.setUniform("Light.Quadratic", 0.0002f);
	this->progIns.setUniform("uRadius", 0.5f);

	this->progSkybox.use();
	GLuint skyboxId = Loader::loadCubeMap("./medias/textures/cubemap_night/night");
	this->m_skybox.setCubeMapId(skyboxId);

	this->setupGBuffer();
	this->setupFBO();
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);

	// create instancing model mats for teapot
	mat4 *modelMats = new mat4[m_teapot_count];
	for (GLuint i = 0; i < m_teapot_count; i++) {
		GLfloat x = (i % 10) * 10.0f - 20.0f;
		GLfloat y = -10.0f;
		GLfloat z = (i / 10) * (-10.0f) + 45.0f;

		mat4 m = glm::translate(mat4(1.0f), vec3(x, y, z));
		m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelMats[i] = m;
	}
	this->m_teapot.loadInstanceMats(modelMats, m_teapot_count);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

// create g-buffer for defer shading
void SceneSSAO::setupGBuffer() {
	glGenFramebuffers(1, &this->gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	unsigned int depthBuf;

	// depth buf
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);

	this->createGBufferTex(GL_RGB16F, gPos, true);
	this->createGBufferTex(GL_RGB16F, gNorm);
	this->createGBufferTex(GL_RGB8, gColor);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPos, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNorm, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);
	
	GLenum drawBufs[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(4, drawBufs);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "SSAO G-Buffer not complete!" << endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// setup FBO for SSAO
void SceneSSAO::setupFBO() {
	// ---setup SSAO pass framebuffer---
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	// SSAO color buffer
	this->createGBufferTex(GL_RGB8, ssaoBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBuf, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "SSAO Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	// ---setup SSAO blur pass framebuffer---
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	// SSAO blur color buffer
	this->createGBufferTex(GL_RGB8, ssaoBlurBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuf, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "SSAO Framebuffer not complete!" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

// compute SSAO kernels and random rotation noises
void SceneSSAO::computeSSAOKernelsAndNoises() {
	// use std:: uniform_real_distribution to uniformly produces random floats in [0, 1)
	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> dis(0.0, 1.0);

	// compute SSAO kernels
	vector<vec3> ssaoKernel;
	int kernelSize = 64;
	for (int i = 0; i < kernelSize; i++) {
		vec3 sample(dis(gen) * 2 - 1.0f, dis(gen) * 2 - 1.0f, dis(gen));
		// normalize to get the sample points on the surface of the hemisphere (because length is 1 after normalize)
		sample = glm::normalize(sample);
		// scale to distribute the sample points within the hemisphere (instead of only on the surface)
		sample *= dis(gen);
		// more samples when closer to the hemisphere origin
		float scale = (float)i / kernelSize;
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;

		ssaoKernel.push_back(sample);
	}
	this->prog.use();
	// set uniform
	for (int i = 0; i < kernelSize; i++) {
		string uname = "ssaoKernel[" + to_string(i) + "]";
		this->prog.setUniform(uname.c_str(), ssaoKernel[i]);
	}

	// compute 4x4 SSAO noises (oriented around tangent-space normals, z = 0.0f)
	vector<vec3> ssaoNoises;
	for (int i = 0; i < 16; i++) {
		vec3 noise(dis(gen) * 2 - 1.0f, dis(gen) * 2 - 1.0f, 0.0f);
		ssaoNoises.push_back(noise);
	}
	// bind noise texture
	glGenTextures(1,&this->noiseTex);
	glBindTexture(GL_TEXTURE_2D, this->noiseTex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, 4, 4);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, &ssaoNoises[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

float SceneSSAO::lerp(float a, float b, float val) {
	return a + val * (b - a);
}

void SceneSSAO::createGBufferTex(GLenum format, GLuint &texId, bool wrap) {
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexStorage2D(GL_TEXTURE_2D, 1, format, this->width, this->height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (wrap) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
}

void SceneSSAO::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), (float)this->width / this->height, 0.1f, 1000.0f);
}

void SceneSSAO::render() {
	// active textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->gPos);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->gNorm);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->gColor);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, this->ssaoBuf);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, this->ssaoBlurBuf);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, this->noiseTex);

	// --------------geometry pass----------------
	this->geometryPass();
	glFlush();
	// --------------End of geometry pass----------------


	// --------------SSAO pass------------------
	this->ssaoPass();
	glFlush();
	// --------------End of SSAO pass------------------


	// --------------SSAO Blur pass------------------
	this->ssaoBlurPass();
	glFlush();
	// --------------End of SSAO Blur pass------------------


	// --------------Lighting pass--------------
	this->lightingPass();
	glFlush();
	// --------------End of lighting pass--------------

	this->renderGUI();

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneSSAO::geometryPass() {
	// bind the g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->geometryPassInx);
	// draw the scene
	this->drawScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneSSAO::ssaoPass() {
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, this->ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glViewport(0, 0, this->width / 2.0f, this->height / 2.0f);
	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->ssaoPassInx);
	// draw the quad with the previous g-buffer and our ssaoKernel and random rotation noise in screen-space

	this->model = glm::mat4(1.0f);
	this->view = glm::mat4(1.0f);
	this->projection = glm::mat4(1.0f);
	this->setMatrices(this->prog.getName());

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneSSAO::ssaoBlurPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, this->ssaoBlurFBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0, width / 2, height / 2);
	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->ssaoBlurPassInx);
	////// draw the quad with previous ssaoFBO output color buffer in screen-space
	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SceneSSAO::lightingPass() {
	// Set the framebuffer to windows default buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLFWwindow *window = SceneManager::getInstance()->getWindow();
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	//printf("%d, %d\n", w, h);
	glViewport(0, 0, width, height);
	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->lightingPassInx);

	// set the lighting-relative uniforms
	this->prog.setUniform("Material.Ka", glm::vec3(0.2, 0.2, 0.2));
	this->prog.setUniform("Material.Ks", glm::vec3(0.5, 0.5, 0.5));
	this->prog.setUniform("Material.Shininess", 50.0f);
	this->prog.setUniform("Light.Position", vec3(this->view * glm::vec4(20.0f, 40.0f, 20.0f, 1.0f)));

	// set the MVP for the rendered quad
	view = mat4(1.0);
	model = mat4(1.0);
	projection = mat4(1.0);
	this->setMatrices(this->prog.getName());

	// draw the single quad to full screen
	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}

void SceneSSAO::drawScene() {
	//// ----- render without instancing -----
	this->prog.use();
	this->prog.setUniform("Material.Kd", glm::vec3(0.8, 0.5, 0.6));
	this->prog.setUniform("ProjectionMatrix", this->projection);
	// render planes
	// bottom plane
	this->model = glm::mat4(1.0f);
	this->model = glm::translate(this->model, glm::vec3(0.0f, -10.0f, 0.0f));
	this->setMatrices(this->prog.getName());
	this->m_plane.render();
	// back plane
	this->model = glm::translate(this->model, glm::vec3(0.0f, 50.0f, -50.0f));
	this->model = glm::rotate(this->model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	this->setMatrices(this->prog.getName());
	this->m_plane.render();

	// ----- render with instancing -----
	this->progIns.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->geometryPassInx);
	this->progIns.setUniform("Material.Kd", glm::vec3(0.8f, 0.8f, 0.3f));

	// render teapot
	this->progIns.setUniform("ProjectionViewMatrix", this->projection * this->view);
	this->progIns.setUniform("ViewMatrix", this->view);
	this->progIns.setUniform("ProjectionMatrix", this->projection);
	this->m_teapot.renderInstances(this->m_teapot_count);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}


void SceneSSAO::resize(int w, int h) {
	this->width = w;
	this->height = h;
	glViewport(0, 0, w, h);
	this->projection = glm::perspective(glm::radians(Camera::getInstance()->getZoom()), (float)w / h, 0.1f, 1000.0f);
}

void SceneSSAO::setMatrices(string progname) {
	ShaderProgram *program = this->progList[progname];
	mat4 mv = this->view * this->model;
	program->setUniform("ModelViewMatrix", mv);
	glm::mat4 norm = glm::transpose(glm::inverse(mv));
	program->setUniform("NormalMatrix",
		glm::mat3(glm::vec3(norm[0]), glm::vec3(norm[1]), glm::vec3(norm[2])));
	program->setUniform("MVP", this->projection * mv);
	//program->setUniform("ProjectionMatrix", this->projection);

}

void SceneSSAO::compileAndLinkShaders() {

	try {
		this->prog.compileShader("./medias/ssaoShader.vert", GLSLShader::VERTEX);
		this->prog.compileShader("./medias/ssaoShader.frag", GLSLShader::FRAGMENT);
		this->prog.link();
		this->progList.insert(std::pair<string, ShaderProgram*>(this->prog.getName(), &this->prog));

		this->progIns.compileShader("./medias/instancingShader.vert", GLSLShader::VERTEX);
		this->progIns.compileShader("./medias/ssaoShader.frag", GLSLShader::FRAGMENT);
		this->progIns.link();
		this->progList.insert(std::pair<string, ShaderProgram*>(this->progIns.getName(), &this->progIns));

		this->progSkybox.compileShader("./medias/skyboxShader.vert", GLSLShader::VERTEX);
		this->progSkybox.compileShader("./medias/skyboxShader.frag", GLSLShader::FRAGMENT);
		this->progSkybox.link();
		this->progList.insert(std::pair<string, ShaderProgram*>(this->progSkybox.getName(), &this->progSkybox));


		GLUtils::checkForOpenGLError(__FILE__, __LINE__);

	}
	catch (ShaderProgramException e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneSSAO::renderGUI() {
	if (this->animating() == true) return;

	ImGui_ImplGlfwGL3_NewFrame("Editor");

	static float f = 0.0f;
	static int counter = 0;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	string rate = "Frame Rate: " + to_string(ImGui::GetIO().Framerate);
	ImGui::Begin("FPS:");
	ImGui::Text("%.1f", ImGui::GetIO().Framerate);               // Display some text (you can use a format strings too)
	//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
	//ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);
	ImGui::End();

	ImGui::Render();
}