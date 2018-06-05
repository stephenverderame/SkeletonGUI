#pragma once
#include "window.h"
#include "glad.h"
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
	~GLWindow();
	void swapBuffers();
	int initGL();
	void repaint();
	void setPaintFunction(glPaintFunc p) { paintCallback = p; }
	void enableDefaultErase(bool defErase) { this->defErase = defErase; }
	inline HDC getDc() { return hdc; }
	inline HGLRC getRc() { return hrc; }
}; 