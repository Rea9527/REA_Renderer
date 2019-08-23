#pragma once

#include <string>
#include <map>
using namespace std;

#include <glad/include/glad/glad.h>
#include <GLFW/glfw3.h>

#include "GLUtils.h"

#include <glm/glm.hpp>


class ShaderProgramException : public runtime_error {
public:
	ShaderProgramException(const string &msg) : runtime_error(msg) {}
};

namespace GLSLShader {
	enum ShaderType {
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		COMPUTE = GL_COMPUTE_SHADER
	};
}

class ShaderProgram {
private:
	string name;

	GLuint handle;
	bool linked;
	map<string, int> uniformLocations;

	GLint getUniformLocation(const char* name);

	bool fileExists(const string &filename);

	string getExtension(const char* filename);

	ShaderProgram(const ShaderProgram &pro) = delete;
	ShaderProgram &operator=(const ShaderProgram &pro) { return *this; }


public:
	ShaderProgram();
	ShaderProgram(string name);
	~ShaderProgram();

	void compileShader(const char* filename);
	void compileShader(const char* filename, GLSLShader::ShaderType type);
	void compileShader(const string &code, GLSLShader::ShaderType type, const char* filename);

	void link();
	void validate();
	void use();

	GLuint getHandle();

	void setName(string mname);
	string getName();

	bool isLinked();

	void bindAttribLocation(GLuint location, const char* name);
	void bindFragDataLocation(GLuint location, const char* name);

	void setUniform(const char* name, float x, float y, float z);
	void setUniform(const char* name, const glm::vec2 &v);
	void setUniform(const char* name, const glm::vec3 &v);
	void setUniform(const char* name, const glm::vec4 &v);
	void setUniform(const char* name, const glm::mat3 &m);
	void setUniform(const char* name, const glm::mat4 &m);
	void setUniform(const char* name, float x);
	void setUniform(const char* name, int x);
	void setUniform(const char* name, bool x);
	void setUniform(const char* name, GLuint x);

	void findUniformLocations();

	void printActiveUniforms();

	void printActiveUniformBlocks();

	void printActiveAttribs();

	const char* getTypeString(GLenum type);



};
