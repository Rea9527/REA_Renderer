#include "SceneDefer.h"


SceneDefer::SceneDefer() : m_teapot(20, mat4(1.0f)), m_teapot_count(100),
							m_sphere(2.0f, 50, 50), m_plane(200, 100, 1, 1),
							prog("deferShader"), progIns("instancingShader") { }

SceneDefer::SceneDefer(int w, int h) : Scene(w, h),
										m_teapot(20, mat4(1.0f)), m_teapot_count(100),
										m_sphere(2.0f, 50, 50), m_plane(200, 100, 1, 1),
										prog("deferShader"), progIns("instancingShader") { }


void SceneDefer::initScene() {
	this->compileAndLinkShaders();

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0, 0.0, 30.0), glm::vec3(0, 1.0, 0), -90, 0);

	// a simple quad for the second pass rendering
	this->quadVAO = BufferObject::genQuadBufferObject();

	this->prog.use();
	GLuint handle = this->prog.getHandle();
	this->geometryPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "geometryPass");
	this->lightingPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "lightingPass");
	this->prog.setUniform("Light.Intensity", vec3(0.9f, 0.9f, 0.9f));

	this->progIns.use();
	this->progIns.setUniform("Light.Intensity", vec3(0.9f, 0.9f, 0.9f));

	this->setupGBuffer();

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


void SceneDefer::setupGBuffer() {
	glGenFramebuffers(1, &this->gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	unsigned int depthBuf;

	// depth buf
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);

	this->createGBufferTex(GL_RGB32F, gPos);
	this->createGBufferTex(GL_RGB32F, gNorm);
	this->createGBufferTex(GL_RGB8, gColor);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPos, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNorm, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColor, 0);

	GLenum drawBufs[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(4, drawBufs);

}

void SceneDefer::createGBufferTex(GLenum format, GLuint &texId) {
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexStorage2D(GL_TEXTURE_2D, 1, format, this->width, this->height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void SceneDefer::update(float dt) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), (float)this->width / this->height, 0.1f, 1000.0f);
}

void SceneDefer::render() {
	// ----------geometry pass------------
	glBindFramebuffer(GL_FRAMEBUFFER, this->gBuffer);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// active textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->gPos);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->gNorm);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->gColor);

	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->geometryPassInx);

	this->drawScene();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// ----------end of geometry pass------------


	// ----------lighting pass------------
	glDisable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->prog.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->lightingPassInx);

	this->prog.setUniform("Material.Ka", glm::vec3(0.1, 0.1, 0.1));
	this->prog.setUniform("Material.Ks", glm::vec3(0.2, 0.2, 0.2));
	this->prog.setUniform("Material.Shininess", 50.0f);
	this->prog.setUniform("Light.Position", this->view * glm::vec4(0.0f, 40.0f, 80.0f, 1.0f));

	view = mat4(1.0);
	model = mat4(1.0);
	projection = mat4(1.0);
	this->setMatrices(this->prog.getName());

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	// ----------end of geometry pass------------

	this->renderGUI();

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneDefer::drawScene() {
	//// ----- render without instancing -----
	this->prog.setUniform("Material.Kd", glm::vec3(0.8, 0.5, 0.6));

	// render planes
	// bottom plane
	this->model = glm::mat4(1.0f);
	this->model = glm::translate(this->model, glm::vec3(0.0f, -10.0f, 0.0f));
	this->setMatrices(this->prog.getName());
	this->m_plane.render();
	// back plane
	this->model = glm::translate(this->model, glm::vec3(0.0f, 10.0f, -50.0f));
	this->model = glm::rotate(this->model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	this->setMatrices(this->prog.getName());
	this->m_plane.render();
	//this->drawScene();

	// ----- render with instancing -----
	this->progIns.use();
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->geometryPassInx);
	this->progIns.setUniform("Material.Kd", glm::vec3(0.8f, 0.8f, 0.3f));

	// render teapot
	this->progIns.setUniform("ProjectionViewMatrix", this->projection * this->view);
	this->progIns.setUniform("ViewMatrix", this->view);
	this->m_teapot.renderInstances(this->m_teapot_count);


	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}


void SceneDefer::resize(int w, int h) {
	this->width = w;
	this->height = h;
	glViewport(0, 0, w, h);
	this->projection = glm::perspective(glm::radians(Camera::getInstance()->getZoom()), (float)w / h, 0.1f, 1000.0f);
}

void SceneDefer::setMatrices(string progname) {
	ShaderProgram *program = this->progList[progname];
	mat4 mv = this->view * this->model;
	program->setUniform("ModelViewMatrix", mv);
	glm::mat4 norm = glm::transpose(glm::inverse(mv));
	program->setUniform("NormalMatrix",
		glm::mat3(glm::vec3(norm[0]), glm::vec3(norm[1]), glm::vec3(norm[2])));
	program->setUniform("MVP", this->projection * mv);

}

void SceneDefer::compileAndLinkShaders() {

	try {
		this->prog.compileShader("./medias/deferShader.vert", GLSLShader::VERTEX);
		this->prog.compileShader("./medias/deferShader.frag", GLSLShader::FRAGMENT);
		this->prog.link();
		this->progList.insert(std::pair<string, ShaderProgram*>(this->prog.getName(), &this->prog));

		this->progIns.compileShader("./medias/instancingShader.vert", GLSLShader::VERTEX);
		this->progIns.compileShader("./medias/deferShader.frag", GLSLShader::FRAGMENT);
		this->progIns.link();
		this->progList.insert(std::pair<string, ShaderProgram*>(this->progIns.getName(), &this->progIns));


		GLUtils::checkForOpenGLError(__FILE__, __LINE__);

	}
	catch (ShaderProgramException e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneDefer::renderGUI() {
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