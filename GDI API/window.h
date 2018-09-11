#pragma once
#include "Canvas.h"
#include "Event.h"
#include <WinUser.h>
struct Dimensions {
	int x;
	int y;
	int width;
	int height;
};
enum WindowProperty {
	cursor,
	menuName,
	background,
	style,
	icon,
	addStyle
};
enum WindowEvent {
	mouseMove,
	mouseClick,
	mouseScroll,
	keyboard,
	resize
};
typedef int(*WindowProc)(int, int, int, int);
#define WND_PROC_HANDLED 1
class Window {
protected:
	HWND window;
	char * className;
	char * windowName;
	WNDCLASSEX wc;
	LRESULT (CALLBACK *wndProc)(HWND, UINT, WPARAM, LPARAM);
	std::vector<EventListener *> list;
	Dimensions wndDimensions;
	Dimensions originalDims;
	Canvas * can;
protected:
	int (*mouseMoveProc)(int x, int y, int key, int null);
	int (*mouseClickProc)(int btn, int x, int y, int null);
	int (*keyboardEventProc)(int e, int key, int strokes, int null);
	int (*resizeProc)(int width, int height, int null, int nill);
	int (*mouseScrollProc)(int x, int y, int keyState, int wheelDelta);
protected:
	virtual int callStdProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
protected:
	static LRESULT CALLBACK windowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l);
public:
	static Window * boundWindow;
public:
	Window(char * name, int style = NULL);
	Window(HWND existingWindow);
	~Window();
	void use() { boundWindow = this; }
	void show(bool v) { ShowWindow(window, v); }
	void addEventListener(EventListener * e);
	void fireEvent(Event e);
	void removeEventListener(EventListener * e);
	void setWindowProperty(WindowProperty w, WNDCLASSEX p);
	void createWindow(int x, int y, int width, int height);
	void setWindowProc(LRESULT (CALLBACK *callback)(HWND, UINT, WPARAM, LPARAM)) {
		wndProc = callback;
	}
	void drawImage(Image * img);
	void deleteImage(Image * img);
	inline void setStyle(int params) { SetWindowLongPtr(window, GWL_STYLE, params); }
	inline void addStyle(int params) { SetWindowLongPtr(window, GWL_STYLE, GetWindowLongPtr(window, GWL_STYLE) | params); }
	void setProc(WindowEvent e, WindowProc p);
	Dimensions getDimensions() { return wndDimensions; }
	Dimensions getOriginalDimensions() { return originalDims; }
	void updateDimensions(int width, int height);
	static Window * getBoundWindow() { return boundWindow; }
	Canvas * getCanvas() { return can; }
	operator HWND() { return window;  }
	HWND getHwnd() { return window; }
};