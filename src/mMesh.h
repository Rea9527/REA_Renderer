#pragma once

#include "TriangleMesh.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;


// TODO change from structure to a individual class
struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shininess;
};


class mMesh : public TriangleMesh {

public:

	vector<Texture> m_textures;
	vector<Material> m_materials;

	mMesh(vector<GLfloat> pos, vector<GLfloat> normals, vector<GLfloat> texcoords, vector<GLuint> indices, vector<Texture> textures, vector<Material> materials);

	void prepare(GLuint handle) const;
	void finish() const;

	void loadInstance(glm::mat4 *modelMatrixs, int count);

};