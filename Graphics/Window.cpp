#include "Window.h"


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Keyboard::getInstance().setKey(key, action != GLFW_RELEASE);
	if (Keyboard::getInstance().getKey(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Mouse::getInstance().setX(xpos);
	Mouse::getInstance().setY(ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	Mouse::getInstance().setButton(button, action != GLFW_RELEASE);
}

Window::Window(int width, int height, const char* title)
{
	this->width = width;
	this->height = height;
	this->title = title;
	if (!init())
	{
		glfwTerminate();
	}
}

Window::~Window()
{
	glfwTerminate();
}

void Window::resetCursor()
{
	glfwSetCursorPos(this->window, width / 2.0, height / 2.0);
}

bool Window::init()
{
	if (!glfwInit())
	{
		std::cerr << "Failed to init GLFW" << std::endl;
		return false;
	}

	this->window = glfwCreateWindow(this->width, this->height, this->title, glfwGetPrimaryMonitor(), nullptr);

	if (!window)
	{
		std::cerr << "Failed to create window" << std::endl;
		return false;
	}

	glfwMakeContextCurrent(this->window);
	glfwSetKeyCallback(this->window, key_callback);
	glfwSetCursorPosCallback(this->window, cursor_position_callback);
	glfwSetMouseButtonCallback(this->window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Failed to init GLEW" << std::endl;
		return false;
	}
	
	return true;
}

void Window::clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::update() const
{
	glfwPollEvents();
	glfwSwapBuffers(this->window);
}

bool Window::closed() const
{
	return glfwWindowShouldClose(this->window);
}