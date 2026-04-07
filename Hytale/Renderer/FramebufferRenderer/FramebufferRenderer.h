/*
 * Copyright (c) FishPlusPlus.
 */
#pragma once

#include "Core.h"

class FramebufferRenderer {
public:
	FramebufferRenderer(Shader* shader);

	void resize(int width, int height);
	void bind();
	void unbind();
	
	//make sure to bind a shader before calling this
	void draw();


private:
	GLint lastFBO = 0;
	GLfloat lastClearColor[4] = { 0, 0, 0, 0 };
	GLint lastVAO = 0;
	GLint lastProgram = 0;
	GLint oldViewport[4] = {0, 0, 0, 0};
	GLboolean depthTest = GL_FALSE;
	GLboolean cullFace = GL_FALSE;
	GLboolean blend = GL_TRUE;

	float vertices[6][4] = {
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,

		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		-1.0f,  1.0f,  0.0f, 1.0f
	};

protected:

	int winW = 0;
	int winH = 0;

	uint32_t fbo;
	uint32_t texture;
	uint32_t vao;
	uint32_t vbo;
	int width;
	int height;
	Shader* shader;

	


};