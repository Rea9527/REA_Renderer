#pragma once

#include "Model.h"


// TODO

class SkModel : public Model {

public:
	SkModel(string path);

	void render(GLuint handle) const;
	void renderInstances(GLuint handle, GLuint count) const;

protected:
	void loadModel(string path);
	//void processNode(aiNode* node, const aiScene* scene);
	//mMesh processMesh(aiMesh* mesh, const aiScene* scene);
	
};