#include "SceneManager.h"


SceneManager* SceneManager::m_sceneManager = NULL;

bool SceneManager::GUI_shown = false;

// camera
GLfloat SceneManager::deltaTime = 0.0f;
GLfloat SceneManager::lastFrame = 0.0f;
GLfloat SceneManager::lastX = 0.0f;
GLfloat SceneManager::lastY = 0.0f;
bool SceneManager::firstMouse = false;
bool SceneManager::keys[1024] = { 0 };


SceneManager::SceneManager(int width, int height, const string title) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	// Set all the required options for GLFW, using OpenGL 4.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// Create window
	this->window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Load the OpenGL functions.
	if (!gladLoadGL()) { exit(-1); }

	// get frame buffer size
	glfwGetFramebufferSize(window, &frame_size_width, &frame_size_height);
	glViewport(0, 0, frame_size_width, frame_size_height);

	// camera
	this->deltaTime = 0.0f;
	this->lastFrame = 0.0f;
	this->lastX = frame_size_width / 2;
	this->lastY = frame_size_height / 2;
	this->firstMouse = true;


	//set keycallback
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// init GUI
	ImGui_ImplGlfwGL3_Init(window, false);

	m_sceneManager = this;
}

SceneManager* SceneManager::getInstance() {
	if (m_sceneManager == NULL) {
		static SceneManager static_instance(800, 600, "Default Window");
		m_sceneManager = &static_instance;

	}
	return m_sceneManager;
}

int SceneManager::run(Scene &scene) {
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
	scene.setDimensions(this->frame_size_width, this->frame_size_height);
	scene.initScene();
	scene.resize(this->frame_size_width, this->frame_size_height);
	//main loop
	Camera::getInstance()->use();
	this->mainLoop(this->window, scene);

	// shut down GUI
	ImGui_ImplGlfwGL3_Shutdown();
	//close window and stop glfw
	glfwTerminate();

	return EXIT_SUCCESS;
}

GLFWwindow* SceneManager::getWindow() {
	assert(this->window);
	return this->window;
}