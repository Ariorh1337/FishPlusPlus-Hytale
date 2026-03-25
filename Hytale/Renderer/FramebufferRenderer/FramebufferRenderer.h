#pragma once

#include "Core.h"

class FramebufferRenderer {
public:
	FramebufferRenderer(Shader* shader);

	void resize(int width, int height);
	void bind();
	void unbind();
	void draw();

private:
	GLint lastFBO;
	GLfloat lastClearColor[4];

	uint32_t fbo;
	uint32_t texture;
	uint32_t vao;
	uint32_t vbo;
	int width;
	int height;
	Shader* shader;

	float vertices[6][4] = {
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,

		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};
};