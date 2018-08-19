#pragma once
#include "resource.h"
#include "window.h"
#include "glad.h"
#include <sstream>
#define GLW_CHOOSE_PIXEL_FAIL 0x100
#define GLW_SET_PIXEL_FAIL 0x200
#define GLW_MAKE_CONTEXT_FAIL 0x300
#define GLW_MAKE_CURRENT_FAIL 0x400
#define GLW_INIT_FAIL 0x500
#define GLW_SUCCESS 0
class GLWindow;
typedef int(CALLBACK *glPaintFunc)(GLWindow * parent);
class GLWindow : public Window {
private:
	HDC hdc;
	HGLRC hrc;
	glPaintFunc paintCallback;
	bool defErase;
protected:
	int callStdProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
public:
	void * paintCallbackData;
public:
	GLWindow(char * name, int style = NULL);
	GLWindow(HWND existingWindow);
	~GLWindow();
	void swapBuffers();
	int initGL();
	void repaint();
	void setPaintFunction(glPaintFunc p) { paintCallback = p; }
	void enableDefaultErase(bool defErase) { this->defErase = defErase; }
	inline HDC getDc() { return hdc; }
	inline HGLRC getRc() { return hrc; }
};
class GLShader {
private:
	unsigned int program;
public:
	void loadProgram(std::string vertexCode, std::string fragmentCode, std::string geometryCode = "", FILE * errstream = stderr);
public:
	GLShader() {};
	GLShader(const char * vertexPath, const char * fragmentPath, const char * geometryPath = nullptr);
	GLShader(int vertexResource, int fragmentResource, int geometryResource = -1, int resourceType = SHADER_RESOURCE, HMODULE module = GetModuleHandle(NULL));
	inline void use() { glUseProgram(program); }
	inline void setInt(char * varName, int val) { glUniform1i(glGetUniformLocation(program, varName), val); }
	inline void setFloat(char * varName, float val) { glUniform1f(glGetUniformLocation(program, varName), val); }
	inline void setVec3(char * varName, float a, float b, float c) { glUniform3f(glGetUniformLocation(program, varName), a, b, c); }
	inline void setMat4(char * varName, float * matrix) { glUniformMatrix4fv(glGetUniformLocation(program, varName), 1, GL_FALSE, matrix); }
	operator GLuint() { return program; }
};