#include "Loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

/*static*/
GLuint Loader::loadTexture(const std::string & fName, GLint wrapMode) {
	int width, height, bytesPerPix;
	unsigned char * data = Loader::loadPixels(fName, width, height, bytesPerPix);


	if (data != nullptr) {
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, width, height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

		stbi_image_free(data);
		return tex;
	}

	return 0;
}

unsigned char *Loader::loadPixels(const std::string &fName, int & width, int & height, int &bytesPerPix) {
	stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load(fName.c_str(), &width, &height, &bytesPerPix, 4);
	return data;
}

GLuint Loader::loadCubeMap(const std::string &baseName) {
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	const char * suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
	GLuint targets[] = {
			GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	};

	stbi_set_flip_vertically_on_load(true);
	GLint w, h, bytesPerPix;

	// Load the first one to get width/height
	std::string texName = baseName + "_" + suffixes[0] + ".png";
	GLubyte * data = Loader::loadPixels(texName.c_str(), w, h, bytesPerPix);

	// Allocate immutable storage for the whole cube map texture
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, w, h);

	glTexSubImage2D(targets[0], 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	// Load the other 5 cube-map faces
	for (int i = 1; i < 6; i++) {
		std::string texName = baseName + "_" + suffixes[i] + ".png";
		GLubyte * data = Loader::loadPixels(texName.c_str(), w, h, bytesPerPix);
		glTexSubImage2D(targets[i], 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}