#pragma once

#include <glad/include/glad/glad.h>
#include <glm/glm.hpp>

namespace BufferObject {

	GLuint genQuadBufferObject(bool normals = false, bool tangents = false);
}