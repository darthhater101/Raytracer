#include "Framebuffer.h"

Framebuffer::Framebuffer()
{
	glGenFramebuffers(1, &framebufferID);
}

void Framebuffer::bindRenderToTexture(int renderedTexture)
{
	bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	unbind();
}

void Framebuffer::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
}

void Framebuffer::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
