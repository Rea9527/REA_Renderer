

#include "shaderProgram.h"
#include <sys/stat.h>

#include <fstream>
#include <sstream>


namespace GLSLShaderInfo {
	struct shader_file_extension {
		const char* ext;
		GLSLShader::ShaderType type;
	};

	struct shader_file_extension extensions[] = {
		{".vert", GLSLShader::VERTEX},
		{".tesc", GLSLShader::TESS_CONTROL},
		{".tese", GLSLShader::TESS_EVALUATION},
		{".geom", GLSLShader::GEOMETRY},
		{".frag", GLSLShader::FRAGMENT},
		{".comp", GLSLShader::COMPUTE}
	};
}


ShaderProgram::ShaderProgram() : handle(0), linked(false) {}

ShaderProgram::ShaderProgram(string pname) : handle(0), linked(false), name(pname) { }
//

ShaderProgram::~ShaderProgram() {
	if (this->handle == 0) return;

	// Query the number of attached shaders
	GLint numShaders = 0;
	glGetProgramiv(handle, GL_ATTACHED_SHADERS, &numShaders);

	// Get the shader names
	GLuint *shaderNames = new GLuint[numShaders];
	glGetAttachedShaders(handle, numShaders, NULL, shaderNames);

	// Delete the shaders
	for (int i = 0; i < numShaders; i++)
		glDeleteShader(shaderNames[i]);

	// Delete the program
	glDeleteProgram(handle);

	delete[] shaderNames;
}

// get the shader type and then compile shader using filename and type
void ShaderProgram::compileShader(const char* filename) {
	int ext_num = sizeof(GLSLShaderInfo::extensions) / sizeof(GLSLShaderInfo::shader_file_extension);

	string ext = this->getExtension(filename);
	// default shader type: vertex
	GLSLShader::ShaderType type = GLSLShader::VERTEX;
	bool match_found = false;

	for (int i = 0; i < ext_num; i++) {

		if (ext == GLSLShaderInfo::extensions[i].ext) {
			type = GLSLShaderInfo::extensions[i].type;
			match_found = true;
			break;
		}
	}

	if (!match_found) {
		string err = "Filename format error or type " + ext + " not exists.";
		throw ShaderProgramException("Compiling shader error: " + err);
	}

	this->compileShader(filename, type);

}

// Get shader code by filename and shader type, then compile shader using filename and type.
void ShaderProgram::compileShader(const char* filename, GLSLShader::ShaderType type) {

	if (!fileExists(filename)) {
		string err = string("File: ") + filename + " not exists.";
		throw ShaderProgramException(err);
	}

	if (this->handle <= 0) {
		this->handle = glCreateProgram();
		if (handle == 0) {
			throw ShaderProgramException("Unable to create shader program.");
		}
	}

	ifstream shader_file(filename, ios::in);
	if (!shader_file) {
		string err = string("Unable to open file ") + filename;
		throw ShaderProgramException(err);
	}

	stringstream ss;
	ss << shader_file.rdbuf();
	shader_file.close();

	this->compileShader(ss.str(), type, filename);
}

// Compile shader using shader code
void ShaderProgram::compileShader(const string &code, GLSLShader::ShaderType type, const char* filename) {

	if (this->handle <= 0) {
		this->handle = glCreateProgram();
		if (handle == 0) {
			throw ShaderProgramException("Unable to create shader program.");
		}
	}

	GLuint shader_handle = glCreateShader(type);
	const GLchar* c_code = code.c_str();
	glShaderSource(shader_handle, 1, &c_code, NULL);

	glCompileShader(shader_handle);
	// Get shader create error
	int result;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		// compile failed
		int length = 0;
		string log_string;
		glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char* c_log = new char[length];
			int written = 0;
			glGetShaderInfoLog(shader_handle, length, &written, c_log);
			log_string = c_log;
			delete[] c_log;
		}

		string err;
		if (filename) {
			err = string(filename) + " : shader compilation failed.\n";
		}
		else {
			err = "Shader compilation failed.\n";
		}
		err += log_string;
		throw ShaderProgramException(err);
	}
	else {
		glAttachShader(this->handle, shader_handle);
	}
}

void ShaderProgram::link() {
	if (this->linked) return;

	if (this->handle <= 0) throw ShaderProgramException("Program has not been created.");

	glLinkProgram(this->handle);

	int status = 0;
	glGetProgramiv(this->handle, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		// compile failed
		int length = 0;
		string log_string;
		glGetProgramiv(this->handle, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char* c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(handle, length, &written, c_log);
			log_string = c_log;
			delete[] c_log;
		}

		throw ShaderProgramException(string("Program linked failed: ") + log_string);
	}
	else {
		this->findUniformLocations();
		this->linked = true;
	}
}

void ShaderProgram::use() {
	if (this->handle <= 0 || linked == false) {
		throw ShaderProgramException("Shader has not been linked.");
	}

	glUseProgram(this->handle);
	
}

void ShaderProgram::findUniformLocations() {
	this->uniformLocations.clear();

	GLint uniformsNum = 0;

	// -------on Windows and Linux, not Apple
	glGetProgramInterfaceiv(this->handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformsNum);

	GLenum properties[] = {GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX};

	for (GLint i = 0; i < uniformsNum; i++) {
		GLint results[4];

		glGetProgramResourceiv(this->handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		// ignore uniform in block
		if (results[3] != -1) continue;

		GLint nameLen = results[0] + 1;
		char *name = new char[nameLen];
		glGetProgramResourceName(this->handle, GL_UNIFORM, i, nameLen, NULL, name);
		uniformLocations[name] = results[2];
		delete[] name;
	}
}


GLuint ShaderProgram::getHandle() {
	return this->handle;
}

bool ShaderProgram::isLinked() {
	return linked;
}

void ShaderProgram::bindAttribLocation(GLuint location, const char* name) {
	glBindAttribLocation(this->handle, location, name);
}

void ShaderProgram::bindFragDataLocation(GLuint location, const char* name) {
	glBindFragDataLocation(this->handle, location, name);
}

void  ShaderProgram::setUniform(const char* name, float x, float y, float z) {
	GLuint loc = this->getUniformLocation(name);
	glUniform3f(loc, x, y, z);
}

void  ShaderProgram::setUniform(const char* name, const glm::vec2 &v) {
	GLuint loc = this->getUniformLocation(name);
	glUniform2f(loc, v.x, v.y);
}

void  ShaderProgram::setUniform(const char* name, const glm::vec3 &v) {
	this->setUniform(name, v.x, v.y, v.z);
}

void  ShaderProgram::setUniform(const char* name, const glm::vec4 &v) {
	GLuint loc = this->getUniformLocation(name);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
}

void  ShaderProgram::setUniform(const char* name, const glm::mat3 &m) {
	GLuint loc = this->getUniformLocation(name);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
}

void  ShaderProgram::setUniform(const char* name, const glm::mat4 &m) {
	GLuint loc = this->getUniformLocation(name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
}

void  ShaderProgram::setUniform(const char* name, float x) {
	GLuint loc = this->getUniformLocation(name);
	glUniform1f(loc, x);
}

void  ShaderProgram::setUniform(const char* name, int x) {
	GLuint loc = this->getUniformLocation(name);
	glUniform1i(loc, x);
}

void  ShaderProgram::setUniform(const char* name, bool x) {
	GLuint loc = this->getUniformLocation(name);
	glUniform1i(loc, x);
}
void  ShaderProgram::setUniform(const char* name, GLuint x) {
	GLuint loc = this->getUniformLocation(name);
	glUniform1ui(loc, x);
}


void ShaderProgram::printActiveUniforms() {
	GLint uniformsNum = 0;

	// -------on Windows and Linux, not Apple
	glGetProgramInterfaceiv(this->handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformsNum);

	GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

	printf("Active Uniforms: \n");
	for (GLint i = 0; i < uniformsNum; i++) {
		GLint results[4];

		glGetProgramResourceiv(this->handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		// ignore uniform in block
		if (results[3] != -1) continue;

		GLint nameLen = results[0] + 1;
		char *name = new char[nameLen];
		glGetProgramResourceName(this->handle, GL_UNIFORM, i, nameLen, NULL, name);
		printf("%-5d %s (%s)", results[2], name, getTypeString(results[1]));
		delete[] name;
	}
}

void ShaderProgram::printActiveUniformBlocks() {
	GLint blocksNum;
	glGetProgramInterfaceiv(this->handle, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &blocksNum);

	GLenum blockProps[] = { GL_NUM_ACTIVE_VARIABLES, GL_NAME_LENGTH };
	GLenum blockIndex[] = { GL_ACTIVE_VARIABLES };
	GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_BLOCK_INDEX };

	for (int block = 0; block < blocksNum; block++) {
		GLint blockInfo[2];
		glGetProgramResourceiv(this->handle, GL_UNIFORM_BLOCK, block, 2, blockProps, 2, NULL, blockInfo);
		GLint uniformsNum = blockInfo[0];

		char* blockName = new char[blockInfo[1] + 1];
		glGetProgramResourceName(this->handle, GL_UNIFORM_BLOCK, block, blockInfo[1] + 1, NULL, blockName);
		printf("Uniform block %s: \n", blockName);
		delete[] blockName;

		GLint *uniformIndicies = new GLint[uniformsNum];
		glGetProgramResourceiv(this->handle, GL_UNIFORM_BLOCK, block, 1, blockIndex, uniformsNum, NULL, uniformIndicies);

		for (int i = 0; i < uniformsNum; i++) {
			GLint index = uniformIndicies[i];

			GLint results[3];
			glGetProgramResourceiv(this->handle, GL_UNIFORM, index, 3, props, 3, NULL, results);

			int nameLen = results[0] + 1;
			char* name = new char[nameLen];
			glGetProgramResourceName(this->handle, GL_UNIFORM, index, nameLen, NULL, name);
			printf("	%s (%s)\n", name, getTypeString(results[1]));
			delete[] name;
		}
		delete[] uniformIndicies;

	}
}

void ShaderProgram::printActiveAttribs() {
	GLint numAttribs;
	glGetProgramInterfaceiv(this->handle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttribs);

	GLenum props[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };
	
	printf("Active attribs: \n");
	for (int i = 0; i < numAttribs; i++) {

		int results[3];
		glGetProgramResourceiv(this->handle, GL_PROGRAM_INPUT, i, 3, props, 3, NULL, results);
		
		int nameLen = results[0];
		char* name = new char[nameLen];
		glGetProgramResourceName(this->handle, GL_PROGRAM_INPUT, i, nameLen, NULL, name);
		printf("%-5d %s (%s)\n", results[2], name, getTypeString(results[1]));
		delete[] name;
	}
}

const char* ShaderProgram::getTypeString(GLenum type) {
	switch (type) {
	case GL_FLOAT:
		return "float";
	case GL_FLOAT_VEC2:
		return "vec2";
	case GL_FLOAT_VEC3:
		return "vec3";
	case GL_FLOAT_VEC4:
		return "vec4";
	case GL_DOUBLE:
		return "double";
	case GL_INT:
		return "int";
	case GL_UNSIGNED_INT:
		return "uint";
	case GL_BOOL:
		return "bool";
	case GL_FLOAT_MAT2:
		return "mat2";
	case GL_FLOAT_MAT3:
		return "mat3";
	case GL_FLOAT_MAT4:
		return "mat4";
	default:
		return "unknown";
	}
}

void ShaderProgram::validate() {
	if (!isLinked()) {
		throw ShaderProgramException("Shader is not linked.");
	}

	glValidateProgram(this->handle);
	GLint status;
	glGetProgramiv(this->handle, GL_VALIDATE_STATUS, &status);

	if (status == GL_FALSE) {
		int length = 0;
		string logString;

		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length);

		if (length > 0) {
			char *c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(handle, length, &written, c_log);
			logString = c_log;
			delete[] c_log;
		}

		throw ShaderProgramException(string("Program failed to validate\n") + logString);
	}
}

int ShaderProgram::getUniformLocation(const char* name) {
	auto loc = uniformLocations.find(name);

	if (loc == uniformLocations.end()) {
		uniformLocations[name] = glGetUniformLocation(this->handle, name);
	}

	return uniformLocations[name];
}

string ShaderProgram::getExtension(const char* filename) {
	string filenameStr(filename);

	size_t index = filenameStr.find_last_of('.');
	if (index != string::npos) {
		return filenameStr.substr(index, string::npos);
	}
	return "";
}

bool ShaderProgram::fileExists(const string &filename) {
	struct stat info;
	int ret = -1;

	ret = stat(filename.c_str(), &info);
	return ret == 0;
}

void ShaderProgram::setName(string mname) {
	this->name = mname;
}

string ShaderProgram::getName() {
	return this->name;
}