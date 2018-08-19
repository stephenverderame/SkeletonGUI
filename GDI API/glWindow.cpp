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
}

GLWindow::GLWindow(HWND existingWindow) : Window(existingWindow)
{
	hdc = GetDC(existingWindow);
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
	hdc = GetDC(window);
	if (hdc == INVALID_HANDLE_VALUE)
		printf("Error invalid window dc");
	int ret = 0;
	PIXELFORMATDESCRIPTOR px = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
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

void GLShader::loadProgram(std::string vertexCode, std::string fragmentCode, std::string geometryCode, FILE * errstream)
{
	unsigned int vs, fs, gs;
	const char * vcode = vertexCode.c_str(), *fcode = fragmentCode.c_str();
	int ret;
	char infoLog[512];
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vcode, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		glGetShaderInfoLog(vs, 512, NULL, infoLog);
		fprintf(errstream, "Error compiling vertex shader: %s", infoLog);
	}

	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fcode, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		glGetShaderInfoLog(fs, 512, NULL, infoLog);
		fprintf(errstream, "Error compiling fragment shader: %s", infoLog);
	}

	if (geometryCode.size() > 1) {
		gs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(gs, 1, &fcode, NULL);
		glCompileShader(gs);
		glGetShaderiv(gs, GL_COMPILE_STATUS, &ret);
		if (!ret) {
			glGetShaderInfoLog(gs, 512, NULL, infoLog);
			fprintf(errstream, "Error compiling geometry shader: %s", infoLog);
		}
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	if (geometryCode.size() > 1) glAttachShader(program, gs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret)
	{
		glGetProgramInfoLog(this->program, 512, NULL, infoLog);
		fprintf(errstream, "Error linking shader program: %s", infoLog);
	}
}

GLShader::GLShader(const char * vertexPath, const char * fragmentPath, const char * geometryPath)
{
	std::string vs, fs, gs;
	std::stringstream buffer;
	std::ifstream reader;
	reader.open(vertexPath);
	if (reader.is_open()) {
		buffer << reader.rdbuf();
		vs = buffer.str();
		buffer.str(std::string());
		reader.close();
	}
	reader.open(fragmentPath);
	if (reader.is_open()) {
		buffer << reader.rdbuf();
		fs = buffer.str();
		buffer.str(std::string());
		reader.close();
	}
	if (geometryPath != nullptr) {
		reader.open(geometryPath);
		if (reader.is_open()) {
			buffer << reader.rdbuf();
			gs = buffer.str();
			buffer.str(std::string());
			reader.close();
		}
		loadProgram(vs, fs, gs);
	}
	else loadProgram(vs, fs);
}

GLShader::GLShader(int vertexResource, int fragmentResource, int geometryResource, int resourceType, HMODULE handle)
{
	gui::Resource v = gui::GUI::loadResource(vertexResource, resourceType, handle);
	gui::Resource f = gui::GUI::loadResource(fragmentResource, resourceType, handle);
	printf("Vdata: %s \n", v.data.c_str());
	if (geometryResource != -1) {
		gui::Resource g = gui::GUI::loadResource(geometryResource, resourceType);
		loadProgram(v.data, f.data, g.data);
	}
	else
		loadProgram(v.data, f.data);
}
