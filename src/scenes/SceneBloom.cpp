#include "SceneBloom.h"
#include <sstream>


SceneBloom::SceneBloom() : plane(20.0f, 10.0f, 1, 1), teapot(12, glm::mat4(1.0)), sphere(2.0f, 50, 50),
								progSkybox("skyboxShader"), progBloom("bloomShader") {}

SceneBloom::SceneBloom(int w, int h) : plane(20.0f, 10.0f, 1, 1), teapot(12, glm::mat4(1.0)), sphere(2.0f, 50, 50),
										progSkybox("skyboxShader"), progBloom("bloomShader"),
										width(w), height(h) {}

SceneBloom::~SceneBloom() {}

void SceneBloom::initScene() {

	this->compileAndLinkShaders();

	this->bloomBufWid = this->width / 2;
	this->bloomBufHei = this->height / 2;

	Camera* camera = Camera::getInstance();
	camera->init(glm::vec3(0, 0.0, 30.0), glm::vec3(0, 1.0, 0), -90, 0);
	
	// create quad for store texture
	this->quadVAO = BufferObject::genQuadBufferObject();

	// setup FBO
	this->setupFBO();

	// set uniforms
	this->progBloom.use();
	// get subroutineIndices
	GLuint handle = this->progBloom.getHandle();
	this->renderPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "renderPass");
	this->brightPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "brightPass");
	this->verGaussPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "verGaussPass");
	this->horGaussPassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "horGaussPass");
	this->tonePassInx = glGetSubroutineIndex(handle, GL_FRAGMENT_SHADER, "tonePass");

	GLint n;
	n = glGetSubroutineUniformLocation(handle, GL_FRAGMENT_SHADER, "bloomPass");

	printf("%d, %d, %d\n", n, this->renderPassInx, this->brightPassInx);

	this->progBloom.setUniform("LumThresh", 1.7f);

	// compute the gaussian weights and set the uniforms
	GLfloat sum, weights[5], sigma2 = 25.0f;
	weights[0] = this->gauss(0, sigma2);
	sum = weights[0];
	for (int i = 1; i < 5; i++) {
		weights[i] = gauss(i, sigma2);
		sum += 2 * weights[i];
	}
	for (int i = 0; i < 5; i++) {
		stringstream uniName;
		uniName << "Weights[" << i << "]";
		this->progBloom.setUniform(uniName.str().c_str(), weights[i] / sum);
	}


	// Set up two sampler objects for linear and nearest filtering
	GLuint samplers[2];
	glGenSamplers(2, samplers);
	linearSampler = samplers[0];
	nearestSampler = samplers[1];

	GLfloat border[] = { 0.0f,0.0f,0.0f,0.0f };
	// Set up the nearest sampler
	glSamplerParameteri(nearestSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(nearestSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameterfv(nearestSampler, GL_TEXTURE_BORDER_COLOR, border);

	// Set up the linear sampler
	glSamplerParameteri(linearSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glSamplerParameteri(linearSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glSamplerParameterfv(linearSampler, GL_TEXTURE_BORDER_COLOR, border);

	// We want nearest sampling except for the last pass.
	glBindSampler(0, nearestSampler);
	glBindSampler(1, nearestSampler);
	glBindSampler(2, nearestSampler);


	this->progSkybox.use();
	// bind cubemap texture
	GLuint skyboxId = Loader::loadCubeMap("./medias/textures/cubemap_night/night");
	this->skybox.setCubeMapId(skyboxId);

	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneBloom::setupFBO() {
	// -----------------------------HDR FBO--------------------------------
	glGenFramebuffers(1, &this->hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, this->hdrFBO);

	// create render texture target
	glGenTextures(1, &this->rttHdr);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->rttHdr);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, this->width, this->height);

	// bind the texture to FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->rttHdr, 0);

	// create depth buffer
	GLuint depthBuf;
	glGenRenderbuffers(1, &depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width, this->height);

	// bind the depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

	// set the targets for fragment outputs
	GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawbufs);

	// -----------------------------Bright and Blur FBO--------------------------------
	glGenFramebuffers(1, &this->blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, this->blurFBO);

	// create two render texture targets for bright pass and blur pass
	glGenTextures(1, &this->rttBright);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->rttBright);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, this->bloomBufWid, this->bloomBufHei);

	glGenTextures(1, &this->rttBlur);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->rttBlur);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, this->bloomBufWid, this->bloomBufHei);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->rttBright, 0);

	glDrawBuffers(1, drawbufs);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void SceneBloom::update(float t) {
	Camera* camera = Camera::getInstance();
	this->view = camera->getViewMat();
	this->projection = glm::perspective(glm::radians(camera->getZoom()), this->width / (float)this->height, 0.1f, 1000.0f);
}

void SceneBloom::render() {
	


	//GLUtils::checkForOpenGLError(__FILE__, __LINE__);

	

	this->renderPass();
	this->computeLogAvgLuminance();
	glFlush();
	this->brightPass();
	glFlush();
	this->verGaussPass();
	glFlush();
	this->horGaussPass();
	glFlush();
	this->tonePass();

	this->renderGUI();

	//GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void SceneBloom::renderPass() {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glViewport(0, 0, this->width, this->height);
	glBindFramebuffer(GL_FRAMEBUFFER, this->hdrFBO);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->renderPassInx);
	
	this->drawScene();

	glFinish();
}
void SceneBloom::brightPass() {
	
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->rttBright, 0);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->brightPassInx);

	glViewport(0, 0, this->bloomBufWid, this->bloomBufHei);
	glDisable(GL_DEPTH_TEST);
	//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	

	this->model = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	projection = glm::mat4(1.0f);
	this->setMatrices(this->progBloom.getName());

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glFinish();

}

void SceneBloom::verGaussPass() {
	// switch the color buffer of blur FBO to blur texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->rttBlur, 0);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->verGaussPassInx);

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void SceneBloom::horGaussPass() {
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// switch the color buffer of blur FBO back to bright texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->rttBright, 0);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->horGaussPassInx);

	glBindVertexArray(this->quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}

void SceneBloom::tonePass() {

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &this->tonePassInx);

	glViewport(0, 0, this->width, this->height);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindSampler(1, linearSampler);

	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindSampler(1, nearestSampler);
}

void SceneBloom::computeLogAvgLuminance() {

	GLfloat *texData = new GLfloat[this->width*this->height * 3];
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->rttHdr);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData);

	float sum = 0.0f;
	for (int i = 0; i < this->width*this->height; i++) {
		float lum = glm::dot(
			glm::vec3(texData[i * 3 + 0], texData[i * 3 + 1], texData[i * 3 + 2]),
			glm::vec3(0.2126f, 0.7152f, 0.0722f)
		);
		sum += logf(lum + 0.00001f);
	}
	sum = expf(sum / (this->width * this->height));
	this->progBloom.setUniform("AvgLum", sum);

	delete[] texData;
}

void SceneBloom::drawScene() {

	this->progBloom.use();

	glm::vec3 intense = glm::vec3(0.9f, 0.9f, 0.9f);
	this->progBloom.setUniform("Light[0].Intensity", intense);
	this->progBloom.setUniform("Light[1].Intensity", intense);
	this->progBloom.setUniform("Light[2].Intensity", intense);

	glm::vec4 lightPos = glm::vec4(0.0f, 4.0f, 6.0f, 1.0f);
	lightPos.x = -8.0f;
	this->progBloom.setUniform("Light[0].Position", lightPos);
	lightPos.x = 0.0f;
	this->progBloom.setUniform("Light[1].Position", lightPos);
	lightPos.x = 8.0f;
	this->progBloom.setUniform("Light[2].Position", lightPos);

	this->progBloom.setUniform("Material.Ka", glm::vec3(0.2, 0.2, 0.2));
	this->progBloom.setUniform("Material.Kd", glm::vec3(1.0, 0.0, 0.0));
	this->progBloom.setUniform("Material.Ks", glm::vec3(0.2, 0.2, 0.2));
	this->progBloom.setUniform("Material.Shininess", 25.0f);

	// render planes
	// bottom plane
	this->model = glm::mat4(1.0f);
	this->model = glm::translate(this->model, glm::vec3(0.0f, -5.0f, 0.0f));
	this->setMatrices(this->progBloom.getName());
	this->plane.render();
	// back plane
	this->model = glm::translate(this->model, glm::vec3(0.0f, 5.0f, -5.0f));
	this->model = glm::rotate(this->model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	this->setMatrices(this->progBloom.getName());
	this->plane.render();
	// top plane
	this->model = glm::mat4(1.0f);
	this->model = glm::translate(this->model, glm::vec3(0.0f, 5.0f, 0.0f));
	this->model = glm::rotate(this->model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	this->setMatrices(this->progBloom.getName());
	this->plane.render();

	// render teapot
	this->progBloom.setUniform("Material.Kd", glm::vec3(0.4f, 0.9f, 0.4f));
	this->model = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, -5.0f, 0.0f));
	this->model = glm::rotate(this->model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	this->setMatrices(this->progBloom.getName());
	this->teapot.render();

	// render sphere
	this->progBloom.setUniform("Material.Kd", glm::vec3(0.4f, 0.4f, 0.9f));
	this->model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, -3.0f, 0.0f));
	this->setMatrices(this->progBloom.getName());
	this->sphere.render();

	//this->progSkybox.use();
	//// render CubeMap
	//this->model = glm::mat4(1.0f);
	//this->view = glm::mat4(glm::mat3(Camera::getInstance()->getViewMat()));
	//this->setMatrices(this->progSkybox.getName());
	//this->skybox.render();

	this->progBloom.use();
}

float SceneBloom::gauss(int x, float sigma2) {
	double coef = 1.0f / sqrt(glm::two_pi<double>() * sigma2);
	double expon = -(x*x) / (2.0f*sigma2);
	return (float)(coef*exp(expon));
}


void SceneBloom::resize(int w, int h) {
	glViewport(0, 0, w, h);
	this->width = w;
	this->height = h;
	// should adjust according to camera TODO
	this->projection = glm::perspective(glm::radians(60.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBloom::setMatrices(string pname) {
	ShaderProgram* program = this->programsList[pname];
	glm::mat4 mv = this->view * this->model;
	program->setUniform("ModelViewMatrix", mv);
	program->setUniform("NormalMatrix",
		glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
	program->setUniform("MVP", this->projection * mv);
}

void SceneBloom::compileAndLinkShaders() {
	try {
		this->progSkybox.compileShader("./medias/skyboxShader.vert", GLSLShader::VERTEX);
		this->progSkybox.compileShader("./medias/skyboxShader.frag", GLSLShader::FRAGMENT);
		this->progSkybox.link();
		this->programsList.insert(std::pair<string, ShaderProgram*>(this->progSkybox.getName(), &this->progSkybox));

		this->progBloom.compileShader("./medias/bloomShader.vert", GLSLShader::VERTEX);
		this->progBloom.compileShader("./medias/bloomShader.frag", GLSLShader::FRAGMENT);
		this->progBloom.link();
		this->programsList.insert(std::pair<string, ShaderProgram*>(this->progBloom.getName(), &this->progBloom));
	}
	catch (ShaderProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);

	}
}
void SceneBloom::renderGUI() {
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