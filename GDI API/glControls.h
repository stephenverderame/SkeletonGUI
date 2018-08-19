#pragma once
#include "glWindow.h"
#include <glm.hpp>
#include <gtc\type_ptr.hpp>
#include <gtc\matrix_transform.hpp>
#include <Psapi.h>
typedef unsigned int glObj;
class stdShader {
private:
	GLShader * shader;
	static stdShader * instance;
	bool inUse = false;
protected:
	stdShader();
	~stdShader();
public:
	static stdShader * getInstance();
	static void del();
	GLShader * operator->() { return shader; }
	GLShader * glShader() { return shader; } 
};
class stdGL {
private:
	int width, height;
public:
	stdGL();
	~stdGL();
};
class GLImage {
private:
	glObj texture, vbo, vao;
	int width, height, components;
	float x = 0, y = 0;
	float xs = 1, ys = 1;
	float r = 0;
	float vertices[12] = {
		0.0, 0.0,
		1.0, 1.0,
		0.0, 1.0,

		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0
	};
public:
	GLImage();
	void del();
	void loadImage(const char * path);
	void translateTo(float x, float y);
	void scale(float xs, float ys);
	void rotate(float r);
	void draw();

};