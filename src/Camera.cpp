
#include "Camera.h"
#include <cstdio>


Camera* Camera::m_camera = NULL;

Camera* Camera::getInstance() {
	if (m_camera == NULL) {
		static Camera static_instance;
		m_camera = &static_instance;

	}
	return m_camera;
}

void Camera::init(glm::vec3 pos, glm::vec3 up, GLfloat yaw, GLfloat pitch) {
	this->STATE = DISABLE;
	this->Pos = pos;
	this->InitUp = up;
	this->Yaw = yaw;
	this->Pitch = pitch;
	this->Speed = 100.0f;
	this->Sensitivity = 0.25f;
	this->Zoom = 45.0f;
	this->updateCameraPos();
}

void Camera::use() {
	this->STATE = ACTIVE;
}

// camera movement -> translate\rotate\zomm
void Camera::translate(CameraMovement moveDir, GLfloat deltaTime) {
	if (STATE == DISABLE) return;
	GLfloat cameraSpeed = this->Speed * deltaTime;
	if (moveDir == CAM_FOWARD)
		this->Pos += cameraSpeed * this->Front;
	else if (moveDir == CAM_BACKWARD)
		this->Pos -= cameraSpeed * this->Front;
	else if (moveDir == CAM_LEFT)
		this->Pos -= glm::normalize(glm::cross(this->Front, this->Up)) * cameraSpeed;
	else if (moveDir == CAM_RIGHT)
		this->Pos += glm::normalize(glm::cross(this->Front, this->Up)) * cameraSpeed;
}

void Camera::rotate(GLfloat xoff, GLfloat yoff) {
	if (STATE == DISABLE) return;
	xoff *= this->Sensitivity;
	yoff *= this->Sensitivity;

	this->Yaw += xoff;
	this->Pitch += yoff;

	if (this->Pitch > 89.0f)
		this->Pitch = 89.0f;
	if (this->Pitch < -89.0f)
		this->Pitch = -89.0f;

	this->updateCameraPos();
}

void Camera::zoom(GLfloat yoff) {
	if (STATE == DISABLE) return;
	if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
		this->Zoom -= yoff;
	if (this->Zoom <= 1.0f)
		this->Zoom = 1.0f;
	if (this->Zoom >= 45.0f)
		this->Zoom = 45.0f;
	
}

void Camera::updateCameraPos() {
	if (this->STATE == DISABLE) return;
	glm::vec3 front;

	front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	front.y = sin(glm::radians(this->Pitch));
	front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
	this->Front = glm::normalize(front);
	this->Right = glm::normalize(glm::cross(this->Front, this->InitUp));
	this->Up = glm::normalize(glm::cross(this->Right, this->Front));

}


glm::mat4 Camera::getViewMat() {
	return glm::lookAt(this->Pos, this->Pos + this->Front, this->Up);
}

glm::vec3 Camera::getPos() {
	return this->Pos;
}

glm::vec3 Camera::getFront() {
	return this->Front;
}

glm::vec3 Camera::getUp() {
	return this->Up;
}

float Camera::getYaw() {
	return this->Yaw;
}

float Camera::getPitch() {
	return this->Pitch;
}

void Camera::setTarget(glm::vec3 target) {
	glm::lookAt(this->Pos, this->Pos + this->Front, this->Up);
}

void Camera::setSpeed(GLfloat speed) {
	this->Speed = speed;
}

void Camera::setSensitivity(GLfloat sensitivity) {
	this->Sensitivity = sensitivity;
}

float Camera::getZoom() {
	return this->Zoom;
}