#include "Graphics/Graphics.h"
#include "Input/Input.h"

#include <iostream>
#include <fstream>
#include <random>

int main()
{
	Window window(1920, 1080, "Raytracer");

	Shader raytracer("Resources/Shaders/quad.vert", "Resources/Shaders/raytracer.frag");
	Shader post("Resources/Shaders/quad.vert", "Resources/Shaders/post.frag");

	std::random_device rd;
	std::mt19937 e2(rd());
	std::uniform_real_distribution<> dist(0.0f, 0.01f);

	glm::vec3 pos = glm::vec3(-5.0, 0.0, 0.0);
	glm::vec3 oldPos = pos;

	glm::vec3 dir = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 oldDir = dir;
	glm::vec3 dirTemp = dir;

	float speed = 0.1;
	float sensivity = 1.5;

	float mouseX = window.getWidth() / 2.0;
	float mouseY = window.getHeight() / 2.0;

	float oldX = mouseX;
	float oldY = mouseY;

	Texture background("Resources/Images/background.jpg");

	Texture firstTexture(window.getWidth(), window.getHeight());
	Framebuffer firstFrame;
	firstFrame.bindRenderToTexture(firstTexture.getID());

	Texture secondTexture(window.getWidth(), window.getHeight());
	Framebuffer secondFrame;
	secondFrame.bindRenderToTexture(secondTexture.getID());

	int framesCounter = 0;
	bool constFrame = false;
	float samples;

	while (!window.closed())
	{
		window.clear();

		dir = glm::vec3(0.0, 0.0, 0.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_W))
			dir += glm::vec3(1.0, 0.0, 0.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_S))
			dir += glm::vec3(-1.0, 0.0, 0.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_D))
			dir += glm::vec3(0.0, 1.0, 0.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_A))
			dir += glm::vec3(0.0, -1.0, 0.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_SPACE))
			dir += glm::vec3(0.0, 0.0, 1.0);
		if (Keyboard::getInstance().getKey(GLFW_KEY_LEFT_CONTROL))
			dir += glm::vec3(0.0, 0.0, -1.0);

		mouseX += Mouse::getInstance().getX() - window.getWidth() / 2.0;
		mouseY -= Mouse::getInstance().getY() - window.getHeight() / 2.0;

		Mouse::getInstance().setX(window.getWidth() / 2.0);
		Mouse::getInstance().setY(window.getHeight() / 2.0);

		window.resetCursor();

		float mx = (mouseX / window.getWidth()) * sensivity;
		float my = (mouseY / window.getHeight()) * sensivity;

		dirTemp.z = dir.z * cos(-my) - dir.x * sin(-my);
		dirTemp.x = dir.z * sin(-my) + dir.x * cos(-my);
		dirTemp.y = dir.y;

		dir.x = dirTemp.x * cos(mx) - dirTemp.y * sin(mx);
		dir.y = dirTemp.x * sin(mx) + dirTemp.y * cos(mx);
		dir.z = dirTemp.z;

		pos += dir * speed;

		if (pos == oldPos && mouseX == oldX && mouseY == oldY)
		{
			samples = 16;
			constFrame = true;
		}
		else
		{
			framesCounter = 0;
			samples = 1;
			oldPos = pos;
			oldX = mouseX;
			oldY = mouseY;
			constFrame = false;
		}

		framesCounter % 2 == 0 ? firstFrame.bind() : secondFrame.bind();

		raytracer.use();
		raytracer.setFloat("u_time", glfwGetTime());
		raytracer.setFloat("u_samples", samples);
		raytracer.setVec2("u_rand1", glm::vec2((float)dist(e2), (float)dist(e2)));
		raytracer.setVec2("u_rand2", glm::vec2((float)dist(e2), (float)dist(e2)));
		raytracer.setVec2("u_res", glm::vec2(window.getWidth(), window.getHeight()));
		raytracer.setVec3("u_pos", glm::vec3(pos.x, pos.y, pos.z));
		raytracer.setVec2("u_mouse", glm::vec2(mx, my));
		raytracer.setInt("u_backgroudTex", 0);
		raytracer.setInt("u_previousFrameTex", 1);
		raytracer.setFloat("u_sameFrames", (1.0f / framesCounter));
		raytracer.setBool("u_constFrame", constFrame);

		glActiveTexture(GL_TEXTURE0 + 0);
		background.bind();

		glActiveTexture(GL_TEXTURE0 + 1);
		framesCounter % 2 == 0 ? secondTexture.bind() : firstTexture.bind();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		raytracer.stopUse();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		post.use();
		post.setInt("u_frameTex", 0);

		glActiveTexture(GL_TEXTURE0 + 0);
		framesCounter % 2 == 0 ? firstTexture.bind() : secondTexture.bind();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		post.stopUse();

		window.update();
		framesCounter++;
	}

	return 0;
}