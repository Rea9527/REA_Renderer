#include "BufferObject.h"


namespace BufferObject {

	GLuint genQuadBufferObject(bool normals, bool tangents) {
		GLuint vao;

		glm::vec3 v1 = glm::vec3(-1.0f, -1.0f, 0.0f);
		glm::vec3 v2 = glm::vec3(1.0f, -1.0f, 0.0f);
		glm::vec3 v3 = glm::vec3(1.0f, 1.0f, 0.0f);
		glm::vec3 v4 = glm::vec3(-1.0f, 1.0f, 0.0f);

		GLfloat verts[] = {
			v1.x, v1.y, v1.z, v2.x, v2.y, v2.z,
			v3.x, v3.y, v3.z, v4.x, v4.y, v4.z
		};

		GLuint indices[] = {
			3, 0, 1,
			3, 1, 2
		};

		glm::vec2 uv1 = glm::vec2(0.0f, 0.0f);
		glm::vec2 uv2 = glm::vec2(1.0f, 0.0f);
		glm::vec2 uv3 = glm::vec2(1.0f, 1.0f);
		glm::vec2 uv4 = glm::vec2(0.0f, 1.0f);

		GLfloat tc[] = {
			uv1.x, uv1.y, uv2.x, uv2.y,
			uv3.x, uv3.y, uv4.x, uv4.y
		};

		// Set up the buffers

		unsigned int indexBuf, posBuf, normBuf, texBuf, tangBuf;

		glGenBuffers(1, &indexBuf);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices, GL_STATIC_DRAW);

		glGenBuffers(1, &posBuf);
		glBindBuffer(GL_ARRAY_BUFFER, posBuf);
		glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

		if (normals) {
			GLfloat norms[] = {
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
			};
			glGenBuffers(1, &normBuf);
			glBindBuffer(GL_ARRAY_BUFFER, normBuf);
			glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), norms, GL_STATIC_DRAW);

		}

		glGenBuffers(1, &texBuf);
		glBindBuffer(GL_ARRAY_BUFFER, texBuf);
		glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), tc, GL_STATIC_DRAW);

		if (tangents) {
			glm::vec3 edge1 = v2 - v1;
			glm::vec3 edge2 = v3 - v1;
			glm::vec3 edge3 = v4 - v1;

			glm::vec2 duv1 = uv2 - uv1;
			glm::vec2 duv2 = uv3 - uv1;
			glm::vec2 duv3 = uv4 - uv1;

			GLfloat f;
			// first triangle
			f = 1.0f / (duv1.x * duv2.y - duv2.x * duv1.y);
			glm::vec3 tvec1;
			tvec1.x = f * (duv2.y * edge1.x - duv1.y * edge2.x);
			tvec1.y = f * (duv2.y * edge1.y - duv1.y * edge2.y);
			tvec1.z = f * (duv2.y * edge1.z - duv1.y * edge2.z);
			tvec1 = glm::normalize(tvec1);
			// second triangle
			f = 1.0f / (duv2.x * duv3.y - duv3.x * duv2.y);
			glm::vec3 tvec2;
			tvec2.x = f * (duv3.y * edge2.x - duv2.y * edge3.x);
			tvec2.y = f * (duv3.y * edge2.y - duv2.y * edge3.y);
			tvec2.z = f * (duv3.y * edge2.z - duv2.y * edge3.z);

			GLfloat tg[] = {
				tvec1.x, tvec1.y, tvec1.z, tvec1.x, tvec1.y, tvec1.z,
				tvec1.x, tvec1.y, tvec1.z, tvec2.x, tvec2.y, tvec2.z
			};
			glGenBuffers(1, &tangBuf);
			glBindBuffer(GL_ARRAY_BUFFER, tangBuf);
			glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), tg, GL_STATIC_DRAW);
		}

		// Set up the vertex array object

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

		glBindBuffer(GL_ARRAY_BUFFER, posBuf);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);  // Vertex position

		if (normals) {
			glBindBuffer(GL_ARRAY_BUFFER, normBuf);
			glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(1);  // Vertex normal
		}

		glBindBuffer(GL_ARRAY_BUFFER, texBuf);
		glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);  // Texture coordinates

		if (tangents) {
			glBindBuffer(GL_ARRAY_BUFFER, tangBuf);
			glVertexAttribPointer((GLuint)3, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(3);  // Vertex tangents
		}

		glBindVertexArray(0);

		return vao;
	}
}