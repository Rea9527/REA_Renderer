#pragma once

#include <iostream>
#include <string>
using namespace std;

// GL CORE
//#include <gl/GL.h>
#include <glad/include/glad/glad.h>

//#include <glad/src/glad.c>
// GLFW
#include <GLFW/glfw3.h>

#include "Scene.h"
#include "Camera.h"
#include "GLUtils.h"
#include "GLGUI.h"


class SceneManager {

public:

	SceneManager(int width, int height, const string title);

	static SceneManager* getInstance();

	int run(Scene &scene);

	GLFWwindow* getWindow();


private:
	
	static SceneManager *m_sceneManager;

	GLFWwindow *window;
	int frame_size_width, frame_size_height;

	// GUI control
	static bool GUI_shown;

	//camera
	static bool keys[1024];
	static GLfloat deltaTime;
	static GLfloat lastFrame;
	static GLfloat lastX;
	static GLfloat lastY;
	static bool firstMouse;

	
	void mainLoop(GLFWwindow* window, Scene &scene) {
		while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			//GLUtils::checkForOpenGLError(__FILE__, __LINE__);
			if (this->GUI_shown == true) {
				scene.animate(false);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
			else {
				scene.animate(true);
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			glfwPollEvents();
			updateMovement();
			scene.update(float(glfwGetTime()));
			scene.render();
			glfwSwapBuffers(window);
		}
	}

	inline static void updateMovement() {
		
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (GUI_shown) return;

		Camera* camera = Camera::getInstance();

		if (keys[GLFW_KEY_W])
			camera->translate(CAM_FOWARD, deltaTime);
		else if (keys[GLFW_KEY_S])
			camera->translate(CAM_BACKWARD, deltaTime);
		else if (keys[GLFW_KEY_A])
			camera->translate(CAM_LEFT, deltaTime);
		else if (keys[GLFW_KEY_D])
			camera->translate(CAM_RIGHT, deltaTime);

	}

	inline static auto key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) -> void {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		if (key == GLFW_KEY_TAB) {
			if (action == GLFW_PRESS)
				GUI_shown = !GUI_shown;
		}

		if (action == GLFW_PRESS) {
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			keys[key] = false;
		}
	}

	inline static auto mouse_callback(GLFWwindow* window, double xpos, double ypos) -> void {

		GLfloat offset_x, offset_y;
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}
		offset_x = xpos - lastX;
		offset_y = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		if (GUI_shown) return;

		Camera* camera = Camera::getInstance();
		camera->rotate(offset_x, offset_y);
	}
	inline static auto scroll_callback(GLFWwindow* window, double xoff, double yoff) -> void {
		if (GUI_shown) return;

		Camera* camera = Camera::getInstance();
		camera->zoom(yoff);
	}


};

