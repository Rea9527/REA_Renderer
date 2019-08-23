
#include "Terrain.h"

#include "stb/stb_image.h"

Terrain::Terrain() : m_vertex_countx(256), m_vertex_countz(256), m_heights(m_vertex_countz*m_vertex_countx, 0) {

	this->m_size = 1600;
	this->m_max_height = 60;
	this->m_max_pcolor_value = 256 * 256 * 256;

	this->m_texId = -1;

	this->generateTerrain();
}

Terrain::Terrain(GLuint size, string heightMapPath, bool multiTex)
	: m_vertex_countx(256), m_vertex_countz(256), m_heights(m_vertex_countz*m_vertex_countx, 0) {
	this->m_size = size;
	this->m_max_height = 100;
	this->m_max_pcolor_value = 256 * 256 * 256;

	this->m_multiTex = multiTex;

	this->generateTerrain(heightMapPath);
}

void Terrain::render() const {

	if (this->vao == 0) return;

	if (this->m_multiTex == true) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->bgTexId);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, this->rTexId);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, this->gTexId);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, this->bTexId);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, this->blendMapId);
	}

	glBindVertexArray(this->vao);
	glDrawElements(GL_TRIANGLES, this->vertex_num, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	if (this->m_multiTex == true) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void Terrain::generateTerrain(string heightMapPath) {

	int width, height, bytesPerPixel;
	unsigned char* data = stbi_load(heightMapPath.c_str(), &width, &height, &bytesPerPixel, 3);
	unsigned char(*pixels)[256][3] = (unsigned char(*)[256][3]) data;
	this->m_vertex_countx = heightMapPath == "" ? this->m_vertex_countx : width;
	this->m_vertex_countz = heightMapPath == "" ? this->m_vertex_countz : height;

	this->m_heights.resize(m_vertex_countx*m_vertex_countz);

	GLuint total_vertex_count = this->m_vertex_countx * this->m_vertex_countz;
	vector<GLfloat> p(total_vertex_count * 3);
	vector<GLfloat> n(total_vertex_count * 3);
	vector<GLfloat> tc(total_vertex_count * 2);
	vector<GLuint> el((this->m_vertex_countx-1) * (this->m_vertex_countz-1) * 6);


	for (int i = 0; i < this->m_vertex_countz; i++) {
		int pointerZ = i * m_vertex_countx;
		for (int j = 0; j < this->m_vertex_countx; j++) {
			int ptr = (pointerZ + j) * 3;
			int ptrh = pointerZ + j;
			// position
			p[ptr + 0] = (GLfloat)j / (m_vertex_countx - 1) * m_size;
			GLfloat height = heightMapPath == "" ? 0.0f : this->computeHeight(i, j, pixels);
			this->m_heights[ptrh] = height;
			p[ptr + 1] = height;
			p[ptr + 2] = (GLfloat)i / (m_vertex_countz - 1) * m_size;
			// normal
			vec3 normal = heightMapPath == "" ? vec3(0.0f, 1.0f, 0.0f) : this->computeNormal(i, j, pixels);
			n[ptr + 0] = normal.x;
			n[ptr + 1] = normal.y;
			n[ptr + 2] = normal.z;
			ptr = (pointerZ + j) * 2;
			// texcoord
			tc[ptr + 0] = (GLfloat)j / (m_vertex_countx - 1);
			tc[ptr + 1] = (GLfloat)i / (m_vertex_countz - 1);
		}
	}

	//for each grid
	int ptr = 0;
	for (int gz = 0; gz < this->m_vertex_countz - 1; gz++) {
		for (int gx = 0; gx < this->m_vertex_countx - 1; gx++) {
			int topLeft = (gz * m_vertex_countx) + gx;
			int topRight = topLeft + 1;
			int bottomLeft = ((gz + 1) * m_vertex_countx) + gx;
			int bottomRight = bottomLeft + 1;
			el[ptr++] = topLeft;
			el[ptr++] = bottomLeft;
			el[ptr++] = topRight;
			el[ptr++] = topRight;
			el[ptr++] = bottomLeft;
			el[ptr++] = bottomRight;
		}
	}

	this->initBuffers(&el, &p, &n, &tc);
}

GLfloat Terrain::getHeight(GLfloat worldX, GLfloat worldZ) {

	GLfloat terrainX = worldX;
	GLfloat terrainZ = worldZ;
	GLfloat gridSquareSizeX = this->m_size / ((GLfloat)this->m_vertex_countx - 1);
	GLfloat gridSquareSizeZ = this->m_size / ((GLfloat)this->m_vertex_countz - 1);

	int gridX = terrainX / gridSquareSizeX;
	int gridZ = terrainZ / gridSquareSizeZ;
	if (gridX < 0 || gridX >= this->m_vertex_countx || gridZ < 0 || gridZ >= this->m_vertex_countz) {
		return 0;
	}

	GLfloat coordX = fmod(terrainX, gridSquareSizeX) / gridSquareSizeX;
	GLfloat coordZ = fmod(terrainZ, gridSquareSizeZ) / gridSquareSizeZ;

	GLfloat result;
	GLint indexZX = gridZ * this->m_vertex_countx + gridX, indexZ1X = (gridZ+1) * this->m_vertex_countx + gridX;
	if (coordX < (1 - coordZ)) {
		result = this->baryCentric(glm::vec3(0, m_heights[indexZX], 0), glm::vec3(1, m_heights[indexZX+1], 0), glm::vec3(0, m_heights[indexZ1X], 1), glm::vec2(coordX, coordZ));
	}
	else {
		result = this->baryCentric(glm::vec3(1, m_heights[indexZX+1], 0), glm::vec3(1, m_heights[indexZ1X+1], 1), glm::vec3(0, m_heights[indexZ1X], 1), glm::vec2(coordX, coordZ));
	}

	return result;
}


float Terrain::baryCentric(vec3 p1, vec3 p2, vec3 p3, vec2 pos) {
	// 1
	//GLfloat det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	//GLfloat l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
	//GLfloat l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
	//GLfloat l3 = 1.0f - l1 - l2;
	//return l1 * p1.y + l2 * p2.y + l3 * p3.y;

	// 2
	vec2 v0 = vec2(p1.x - p3.x, p1.z - p3.z), v1 = vec2(p2.x - p3.x, p2.z - p3.z), v2 = vec2(pos.x - p3.x, pos.y - p3.z);
	GLfloat d00 = glm::dot(v0, v0);
	GLfloat d01 = glm::dot(v0, v1);
	GLfloat d11 = glm::dot(v1, v1);
	GLfloat d20 = glm::dot(v2, v0);
	GLfloat d21 = glm::dot(v2, v1);
	
	GLfloat denom = d00 * d11 + d01 * d01;
	GLfloat v = (d11 * d20 - d01 * d21) / denom;
	GLfloat w = (d00 * d21 - d01 * d20) / denom;
	GLfloat u = 1.0f - v - w;
	return u * p1.y + v * p2.y + w * p3.y;
}



float Terrain::computeHeight(int z, int x, unsigned char(*pixels)[256][3]) {

	if (z < 0 || z >= m_vertex_countz || x < 0 || x >= m_vertex_countx) return 0;

	//int k = z * x * 3 + x * 3;
	//float r = (float)pixels[k + 0];
	//float g = (float)pixels[k + 1];
	//float b = (float)pixels[k + 2];
	float r = (float)pixels[z][x][0];
	float g = (float)pixels[z][x][1];
	float b = (float)pixels[z][x][2];

	float hei = ((r * 256 + g) * 256 + b);
	hei /= this->m_max_pcolor_value;
	hei *= this->m_max_height;

	return hei;
	
}

vec3 Terrain::computeNormal(int z, int x, unsigned char(*pixels)[256][3]) {
	float heightL = computeHeight(z, x-1, pixels);
	float heightR = computeHeight(z, x+1, pixels);
	float heightU = computeHeight(z-1, x, pixels);
	float heightD = computeHeight(z+1, x, pixels);

	glm::vec3 normal = glm::vec3(heightL - heightR, 2.0f, heightD - heightU);
	normal = glm::normalize(normal);
	return normal;
}

GLuint Terrain::getTextureId() {
	return this->m_texId;
}

GLuint Terrain::getSize() {
	return this->m_size;
}


void Terrain::setMultiTexIds(GLuint bgId, GLuint rId, GLuint gId, GLuint bId, GLuint blendId) {
	this->bgTexId = bgId;
	this->rTexId = rId;
	this->gTexId = gId;
	this->bTexId = bId;
	this->blendMapId = blendId;
}

GLuint Terrain::_bgTexId() {
	return this->bgTexId;
}
GLuint Terrain::_rTexId() {
	return this->rTexId;
}
GLuint Terrain::_gTexId() {
	return this->gTexId;
}
GLuint Terrain::_bTexId() {
	return this->bTexId;
}
GLuint Terrain::_blendMapId() {
	return this->blendMapId;
}