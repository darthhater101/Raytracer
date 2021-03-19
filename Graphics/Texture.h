#pragma once
#include <GL/glew.h>


class Texture
{
private:
	unsigned int textureID;

	int width;
	int height;
	int comp;
	unsigned char* data = nullptr;

public:
	Texture(const char* fileName = nullptr);
	Texture(int width, int height);
	~Texture();

	void bind();
	void unbind();

	unsigned int getID()
	{
		return textureID;
	}
};

