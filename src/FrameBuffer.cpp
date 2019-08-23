
#include "FrameBuffer.h"


// set current active texture to texture id 2
unsigned int FrameBuffer::s_curActiveTexId = GL_TEXTURE2;


FrameBuffer::FrameBuffer() {}

FrameBuffer::FrameBuffer(GLuint w, GLuint h, bool useDepth, GLuint rrtFormat)
	: m_width(w), m_height(h), m_handle(0), m_depthHandle(0), m_hasDepthBuf(useDepth),
	m_rttFormat(rrtFormat) {

	this->setupFBO();
}

FrameBuffer::~FrameBuffer() {
	glDeleteFramebuffers(1, &this->m_handle);
	if (this->m_hasDepthBuf) glDeleteBuffers(1, &this->m_depthHandle);
}

void FrameBuffer::init(GLuint w, GLuint h, bool useDepth, GLuint rttFormat) {
	this->m_width = w;
	this->m_height = h;

	this->m_depthHandle = 0;
	this->m_handle = 0;

	this->m_hasDepthBuf = useDepth;
	this->m_rttFormat = rttFormat;

	this->setupFBO();
}

void FrameBuffer::setupFBO() {

	glGenFramebuffers(1, &this->m_handle);
	this->bind();

	// create render texture target
	glGenTextures(1, &this->m_rttHandle);
	glBindTexture(GL_TEXTURE_2D, this->m_rttHandle);

	// texture parameter
	glTexStorage2D(GL_TEXTURE_2D, 1, this->m_rttFormat, this->m_width, this->m_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->m_rttHandle, 0);

	if (m_hasDepthBuf) {
		// create depth buffer object
		GLuint depthBuf;
		glGenRenderbuffers(1, &depthBuf);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->m_width, this->m_height);

		// bind the depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

	}

	// set the target for the fragment shader outputs
	GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBufs);

	this->unbind();
}

void FrameBuffer::bind() {

	glBindFramebuffer(GL_FRAMEBUFFER, this->m_handle);
}

void FrameBuffer::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}