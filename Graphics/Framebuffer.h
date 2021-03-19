#pragma once
#include <GL/glew.h>

class Framebuffer
{
private:
	unsigned int framebufferID;

public:
	Framebuffer();

	void bindRenderToTexture(int renderedTexture);

	void bind();
	void unbind();
};

