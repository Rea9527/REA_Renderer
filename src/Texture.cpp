#include "texture.h"




Texture::Texture() : id(0), type(""), path("") { }

Texture::Texture(GLuint id, std::string type, std::string path) : id(id), type(type), path(path) { }


