#include "Model.h"
#include <iostream>

Model::Model(string path) {
	this->loadModel(path);
}

void Model::render(GLuint handle) const {

	for (GLuint i = 0; i < this->m_meshes.size(); i++) {
		this->m_meshes[i].prepare(handle);
		this->m_meshes[i].render();
		this->m_meshes[i].finish();
	}
}

void Model::renderInstances(GLuint handle, GLuint count) const {

	for (GLuint i = 0; i < this->m_meshes.size(); i++) {
		this->m_meshes[i].prepare(handle);
		this->m_meshes[i].renderInstances(count);
		this->m_meshes[i].finish();
	}
}

void Model::loadInstanceTranslationMat(glm::mat4 *modelMats, int count) {
	for (GLuint i = 0; i < this->m_meshes.size(); i++) {
		this->m_meshes[i].loadInstance(modelMats, count);
	}
}

void Model::loadModel(string path) {
	Assimp::Importer importor;
	const aiScene* scene = importor.ReadFile(path, aiProcess_FlipUVs | aiProcess_Triangulate);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		cerr << "ERROR::ASSIMP::" << importor.GetErrorString() << endl;
		return;
	}

	this->m_dir = path.substr(0, path.find_last_of("/"));
	this->processNode(scene->mRootNode, scene);

}

void Model::processNode(aiNode *node, const aiScene *scene) {

	for (GLuint i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->m_meshes.push_back(this->processMesh(mesh, scene));
	}


	for (GLuint i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], scene);
	}
}

mMesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	vector<GLfloat> p(3 * mesh->mNumVertices);
	vector<GLfloat> n(3 * mesh->mNumVertices);
	vector<GLfloat> tc(2 * mesh->mNumVertices);
	vector<GLuint> el(6 * (mesh->mNumVertices-1) * (mesh->mNumVertices - 1));
	vector<Material> materials;
	vector<Texture> textures;

	GLuint vindex = 0;
	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		GLuint j = i * 3;
		// position
		p[j + 0] = mesh->mVertices[i].x;
		p[j + 1] = mesh->mVertices[i].y;
		p[j + 2] = mesh->mVertices[i].z;
		// normal
		n[j + 0] = mesh->mNormals[i].x;
		n[j + 1] = mesh->mNormals[i].y;
		n[j + 2] = mesh->mNormals[i].z;
		// texcoords
		j = i * 2;
		if (mesh->mTextureCoords[0]) {
			tc[j + 0] = mesh->mTextureCoords[0][i].x;
			tc[j + 1] = mesh->mTextureCoords[0][i].y;
		}
		else {
			tc[j + 0] = 0.0f;
			tc[j + 1] = 0.0f;
		}
	}

	// indicies
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++) {
			el.push_back(face.mIndices[j]);
		}
	}

	// materials
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* AiMaterial = scene->mMaterials[mesh->mMaterialIndex];

		aiColor4D color;
		Material material;
		material.shininess = 32.0f;
		if (AI_SUCCESS == aiGetMaterialColor(AiMaterial, AI_MATKEY_COLOR_AMBIENT, &color)) {
			material.ambient.r = color.r;
			material.ambient.g = color.g;
			material.ambient.b = color.b;
		}

		if (AI_SUCCESS == aiGetMaterialColor(AiMaterial, AI_MATKEY_COLOR_DIFFUSE, &color)) {
			material.diffuse.r = color.r;
			material.diffuse.g = color.g;
			material.diffuse.b = color.b;
		}

		if (AI_SUCCESS == aiGetMaterialColor(AiMaterial, AI_MATKEY_COLOR_SPECULAR, &color)) {
			material.specular.r = color.r;
			material.specular.g = color.g;
			material.specular.b = color.b;
		}

		materials.push_back(material);



		vector<Texture> diffuseMaps = this->loadMaterialTextures(AiMaterial,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = this->loadMaterialTextures(AiMaterial,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		vector<Texture> heightMaps = this->loadMaterialTextures(AiMaterial,
			aiTextureType_HEIGHT, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		vector<Texture> normalMaps = this->loadMaterialTextures(AiMaterial,
			aiTextureType_NORMALS, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	}

	return mMesh(p, n, tc, el, textures, materials);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
	vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++) {
		aiString path;
		mat->GetTexture(type, i, &path);
		GLboolean skip = false;
		for (GLuint j = 0; j < this->m_textures.size(); j++)
		{
			if (std::strcmp(this->m_textures[j].path.c_str(), path.C_Str()) == 0)
			{
				textures.push_back(this->m_textures[j]);
				skip = true;
				break;
			}
		}
		if (!skip) {
			Texture texture;
			string spath = string(path.C_Str());
			texture.id = Loader::loadTexture(this->m_dir + '/' + spath, GL_REPEAT);
			texture.type = typeName;
			texture.path = spath;
			textures.push_back(texture);
			this->m_textures.push_back(texture);
		}
	}

	return textures;
}