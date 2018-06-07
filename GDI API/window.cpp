#include "window.h"
Window * Window::boundWindow = nullptr;
int Window::callStdProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
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
LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l)
{
	for (auto it = boundWindow->list.begin(); it != boundWindow->list.end(); it++) {
		if ((*(*it))(msg, EventParams(l, w)))
			return TRUE;
	}
	if (boundWindow->wndProc != nullptr)
		if(boundWindow->wndProc(hwnd, msg, w, l) == 1) return TRUE;
	if (boundWindow->callStdProc(hwnd, msg, w, l)) return TRUE;
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		boundWindow->updateDimensions(LOWORD(l), HIWORD(l));
		gui::GUI::setDimensions(LOWORD(l), HIWORD(l));
		break;
	case WM_PAINT:
		if (boundWindow->can != nullptr) 
			boundWindow->can->draw();
		break;
	}
	if (boundWindow->can != nullptr)
		boundWindow->can->handleMessages(msg, w, l);
	return DefWindowProc(hwnd, msg, w, l);
}
void Window::setWindowProperty(WindowProperty w, WNDCLASSEX p)
{
	switch (w) {
	case cursor:
		wc.hCursor = p.hCursor;
		break;
	case menuName:
		wc.lpszMenuName = p.lpszMenuName;
		break;
	case background:
		wc.hbrBackground = p.hbrBackground;
		break;
	case style:
		wc.style = p.style;
		break;
	case WindowProperty::addStyle:
		wc.style |= p.style;
		break;
	case icon:
		wc.hIcon = p.hIcon;
		wc.hIconSm = p.hIconSm;
		break;
	}
}
void Window::createWindow(int x, int y, int width, int height)
{
	RECT size = { 0, 0, width, height };
	AdjustWindowRect(&size, WS_OVERLAPPED, FALSE);
	RegisterClassEx(&wc);
	window = CreateWindowEx(NULL, wc.lpszClassName, windowName, WS_OVERLAPPEDWINDOW | wc.style, x, y, size.right - size.left, size.bottom - size.top, NULL, NULL, NULL, NULL);
	ShowWindow(window, true);
	wndDimensions.width = size.right - size.left;
	wndDimensions.height = size.bottom - size.top;
	originalDims = wndDimensions;
	can = new Canvas(window);
}
void Window::drawImage(Image * img)
{
	can->addImage(img);
	RECT rect;
	GetClientRect(window, &rect);
	InvalidateRect(window, &rect, TRUE);
}
void Window::deleteImage(Image * img)
{
	if (can != nullptr)
		can->removeImage(img);
}
void Window::setProc(WindowEvent e, WindowProc p)
{
	switch (e) {
	case mouseMove:
		mouseMoveProc = p;
		break;
	case mouseClick:
		mouseClickProc = p;
		break;
	case keyboard:
		keyboardEventProc = p;
		break;
	case resize:
		resizeProc = p;
		break;
	case mouseScroll:
		mouseScrollProc = p;
		break;
	}
}
void Window::updateDimensions(int width, int height)
{
	wndDimensions.width = width;
	wndDimensions.height = height;
}
Window::Window(char * name, int style)
{
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | style;
	wc.lpfnWndProc = windowProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "defWindowClass";
	windowName = name;


}
Window::~Window()
{
	for (auto it = list.begin(); it != list.end(); it++) {
		delete (*it);
	}
	if (can != nullptr)
		delete can;
	DestroyWindow(window);
}
void Window::addEventListener(EventListener * e)
{
	list.push_back(e);
}
void Window::fireEvent(Event e)
{
	SendMessage(window, e.getEventType(), e.getParams().getParam3(), e.getParams().getParaml());
}
void Window::removeEventListener(EventListener * e)
{
	list.erase(std::find(list.begin(), list.end(), e));
}