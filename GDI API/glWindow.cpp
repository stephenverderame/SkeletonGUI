#include "glWindow.h"

int GLWindow::callStdProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	if (paintCallback != nullptr)
		if (paintCallback(this)) return WND_PROC_HANDLED;
	if (defErase == false && msg == WM_ERASEBKGND) return WND_PROC_HANDLED;
	if (mouseMoveProc != nullptr && msg == WM_MOUSEMOVE)
		if (mouseMoveProc(LOWORD(l), HIWORD(l), w, 0) == WND_PROC_HANDLED) return WND_PROC_HANDLED;
	if (mouseClickProc != nullptr && (msg >= 513 && msg <= 521))
		if (mouseClickProc(msg, LOWORD(l), HIWORD(l), w)) return WND_PROC_HANDLED;
	if (mouseScrollProc != nullptr && msg == WM_MOUSEHWHEEL)
		if (mouseScrollProc(LOWORD(l), HIWORD(l), GET_KEYSTATE_WPARAM(w), GET_WHEEL_DELTA_WPARAM(w))) return WND_PROC_HANDLED;
	if (keyboardEventProc != nullptr && (msg == WM_KEYDOWN || msg == WM_KEYUP))
		if (keyboardEventProc(msg, w, l, 0)) return WND_PROC_HANDLED;
	if (resizeProc != nullptr && msg == WM_SIZE)
		if (resizeProc(LOWORD(l), HIWORD(l), 0, 0)) return WND_PROC_HANDLED;
}

GLWindow::GLWindow(char * name, int style) : Window(name, style), defErase(true)
{
	hdc = GetDC(window);
}

GLWindow::~GLWindow()
{
	ReleaseDC(window, hdc);
	wglDeleteContext(hrc);
}

void GLWindow::swapBuffers()
{
	SwapBuffers(hdc);
}
int GLWindow::initGL()
{
	PIXELFORMATDESCRIPTOR px;
	int ret = 0;
	ZeroMemory(&px, sizeof(px));
	px.nSize = sizeof(px);
	px.nVersion = 1;
	px.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	px.iPixelType = PFD_TYPE_RGBA;
	px.cColorBits = 32;
	px.cStencilBits = 32;
	px.cDepthBits = 32;
	px.cAlphaBits = 8;
	ret = ChoosePixelFormat(hdc, &px);
	if (!ret) return GLW_CHOOSE_PIXEL_FAIL;
	if (SetPixelFormat(hdc, ret, &px) == 0) return GLW_SET_PIXEL_FAIL;
	DescribePixelFormat(hdc, ret, sizeof(px), &px);

	hrc = wglCreateContext(hdc);
	if (hrc == NULL) return GLW_MAKE_CONTEXT_FAIL;
	if (wglMakeCurrent(hdc, hrc) == FALSE) return GLW_MAKE_CURRENT_FAIL;
	if (!gladLoadGL()) return GLW_INIT_FAIL;
	return GLW_SUCCESS;
}

void GLWindow::repaint()
{
	RECT r;
	GetClientRect(window, &r);
	InvalidateRect(window, &r, TRUE);
}
