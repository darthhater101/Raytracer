#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "../Input/Input.h"


class Window
{
protected:

	int width;
	int height;
	const char* title;

	GLFWwindow* window;

public:
	Window(int width, int height, const char* title);
	~Window();

	void update() const;
	void clear() const;
	bool closed() const;

	inline int getWidth()
	{
		return width;
	}

	inline int getHeight()
	{
		return height;
	}

	void resetCursor();

protected:
	bool init();

};

