#pragma once

#include <glad/include/glad/glad.h>

#include "Scene.h"

class FrameBuffer {

public:

	FrameBuffer();
	FrameBuffer(GLuint w, GLuint h, bool useDepth, GLuint rrtFormat = GL_RGBA8);
	~FrameBuffer();

	void init(GLuint w, GLuint h, bool useDepth, GLuint rrtFormat = GL_RGBA8);

	// return render texture target handle
	GLuint getRTTHandle() const { return this->m_rttHandle; }

	// bind FBO for rendering
	void bind();

	// unbind FBO, set back to default frame buffer
	void unbind();

	static unsigned int s_curActiveTexId;

private:

	GLuint m_handle;
	GLuint m_rttHandle;
	GLuint m_depthHandle;
	GLuint m_rttFormat;

	GLuint m_width;
	GLuint m_height;

	bool m_hasDepthBuf;

	// set up FBO
	void setupFBO();

};