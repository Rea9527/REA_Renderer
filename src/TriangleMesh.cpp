
#include "TriangleMesh.h"


void TriangleMesh::initBuffers(vector<GLuint> *indices, vector<GLfloat> *points,
	vector<GLfloat> *normals, vector<GLfloat> *texcoords, vector<GLfloat> *tangents) {

	if (indices == nullptr || points == nullptr || normals == nullptr) return;

	this->vertex_num = (GLuint)indices->size();

	// create vao
	GLuint indexBuf = 0, posBuf = 0, normBuf = 0, texcBuf = 0, tangBuf = 0;

	glGenBuffers(1, &indexBuf);
	this->buffers.push_back(indexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), indices->data(), GL_STATIC_DRAW);

	glGenBuffers(1, &posBuf);
	this->buffers.push_back(posBuf);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glBufferData(GL_ARRAY_BUFFER, points->size() * sizeof(GLfloat), points->data(), GL_STATIC_DRAW);

	glGenBuffers(1, &normBuf);
	this->buffers.push_back(normBuf);
	glBindBuffer(GL_ARRAY_BUFFER, normBuf);
	glBufferData(GL_ARRAY_BUFFER, normals->size() * sizeof(GLfloat), normals->data(), GL_STATIC_DRAW);

	if (texcoords != nullptr) {
		glGenBuffers(1, &texcBuf);
		this->buffers.push_back(texcBuf);
		glBindBuffer(GL_ARRAY_BUFFER, texcBuf);
		glBufferData(GL_ARRAY_BUFFER, texcoords->size() * sizeof(GLfloat), texcoords->data(), GL_STATIC_DRAW);
	}

	if (tangents != nullptr) {
		glGenBuffers(1, &tangBuf);
		this->buffers.push_back(tangBuf);
		glBindBuffer(GL_ARRAY_BUFFER, tangBuf);
		glBufferData(GL_ARRAY_BUFFER, tangents->size() * sizeof(GLfloat), tangents->data(), GL_STATIC_DRAW);
	}

	// bind vao
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);

	// bind indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	// bind points
	glBindBuffer(GL_ARRAY_BUFFER, posBuf);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	//bind normals
	glBindBuffer(GL_ARRAY_BUFFER, normBuf);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// bind texcoords
	if (texcoords != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, texcBuf);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
	}

	// bind tangents
	if (tangents != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, tangBuf);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);
	}

	glBindVertexArray(0);
}

void TriangleMesh::loadInstanceMats(glm::mat4 *modelMats, GLuint count) const {
	GLuint instanceBuf;
	glGenBuffers(1, &instanceBuf);
	glBindBuffer(GL_ARRAY_BUFFER, instanceBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * count, &modelMats[0], GL_STATIC_DRAW);

	glBindVertexArray(this->vao);
	GLsizei vec4Size = sizeof(glm::vec4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(vec4Size));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
}


void TriangleMesh::render() const {
	if (this->vao == 0) return;

	glBindVertexArray(this->vao);
	glDrawElements(GL_TRIANGLES, this->vertex_num, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


void TriangleMesh::renderInstances(GLuint count) const {

	if (this->vao == 0) return;

	glBindVertexArray(this->vao);
	glDrawElementsInstanced(GL_TRIANGLES, GLsizei(this->vertex_num), GL_UNSIGNED_INT, 0, count);
	glBindVertexArray(0);
}


TriangleMesh::~TriangleMesh() {
	//this->deleteBuffers();
}


void TriangleMesh::deleteBuffers() {
	if (this->buffers.size() > 0) {
		glDeleteBuffers((GLsizei)this->buffers.size(), this->buffers.data());
		this->buffers.clear();
	}

	if (this->vao != 0) {
		glDeleteVertexArrays(1, &this->vao);
		this->vao = 0;
	}
}