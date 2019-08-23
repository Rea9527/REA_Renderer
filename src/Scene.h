#pragma once

#include <glad/include/glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"


class Scene {

public:

	Scene() : width(800), height(600) {}
	Scene(int w, int h) 
		: width(w), height(h) {}

	virtual ~Scene() {}

	void setDimensions(int w, int h) {
		this->width = w;
		this->height = h;
	}

	// Compile the shader, create data, bind data to buffer...
	virtual void initScene() = 0;
	//
	virtual void update(float t) = 0;
	//
	virtual void render() = 0;
	//
	virtual void resize(int, int) = 0;

	void animate(bool value) { m_animate = value; }
	bool animating() { return m_animate; }

protected:
	glm::mat4 model, view, projection;

	int width;
	int height;

	bool m_animate;

};