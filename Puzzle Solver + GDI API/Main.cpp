#define WINVER 0x501 
#define GUI_VISUAL_STYLES_6
#include <thread>
#include <mutex>
#include <Page.h>
#include <window.h>
#include <Dialog.h>
#include "res.h"
#include <ObjIdl.h>
#include <time.h>
#include "Grid.h"
#include "OCR.h"
using namespace gui;
MainPage * mainPage;
Image * img;
Window display("Puzzle Solver");
bool crop = false, showFinal = false;
bool selecting = false;
int sClicks = 0;
int clicks = 0;
POINT start, finish;
POINT position_1 = { 0, 0 }, position_2 = { 0, 0 };
clock_t lastTime = 0;
int * buffer;
std::mutex sMu;
bool solving = false;
bool invalidate = false;
std::mutex iMu;
HCURSOR cursors[4];
int currentCursor = 0;
#define CURSOR_NORMAL 0
#define CURSOR_WAIT 1
#define CURSOR_DRAW 2
#define CURSOR_SELECT 3
void calculatePath() {
	sMu.lock();
	solving = true;
	sMu.unlock();
	Grid g(img->getWidth(), img->getHeight(), buffer);
	if (position_1.x != position_2.x && position_1.y != position_2.y)
		g.setBound({ position_1.x, position_1.y }, { position_2.x, position_2.y });
	auto path = g.search({ start.x, start.y }, { finish.x, finish.y });
	while (!path.empty()) {
		pos color = path.top();
		img->setPixel(color.x, color.y, Color{ 0, 0, 255 });
		path.pop();
	}
	sMu.lock();
	solving = false;
	sMu.unlock();
	std::lock_guard<std::mutex> lock(iMu);
	invalidate = true;
}
LRESULT CALLBACK customProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	std::vector<Image *> * images = Window::boundWindow->getCanvas()->getImages();
	HDC hdc;
	PAINTSTRUCT paint;
	if (msg == WM_KEYDOWN && !crop) {
		switch (w) {
		case VK_LEFT:
			for (auto it = images->begin(); it != images->end(); it++) {
				(*it)->setPosition((*it)->getX() - 15, (*it)->getY());
			}
			break;
		case VK_RIGHT:
			for (auto it = images->begin(); it != images->end(); it++) {
				(*it)->setPosition((*it)->getX() + 15, (*it)->getY());
			}
			break;
		case VK_UP:
			for (auto it = images->begin(); it != images->end(); it++) {
				(*it)->setPosition((*it)->getX(), (*it)->getY() - 15);
			}
			break;
		case VK_DOWN:
			for (auto it = images->begin(); it != images->end(); it++) {
				(*it)->setPosition((*it)->getX(), (*it)->getY() + 15);
			}
			break;
		}
	}
	if (msg == WM_LBUTTONDOWN && crop && clock() / CLOCKS_PER_SEC > lastTime + 1) {
		lastTime = clock() / CLOCKS_PER_SEC;
		if (!clicks) {
			clicks++;
			position_1 = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
			hdc = GetDC(hwnd);
			SetPixel(hdc, position_1.x, position_1.y, RGB(255, 0, 0));
			ReleaseDC(hwnd, hdc);
		}
		else {
			crop = false;
			position_2 = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
			showFinal = true;
			currentCursor = CURSOR_NORMAL;
			clicks = 0;
		}
	}
	else if (msg == WM_RBUTTONDOWN && (crop || showFinal)) {
		crop = false;
		clicks = 0;
		showFinal = false;
		RECT r;
		GetClientRect(hwnd, &r);
		InvalidateRect(hwnd, &r, TRUE);
		currentCursor = CURSOR_NORMAL;
		position_1 = { 0, 0 };
		position_2 = { 0, 0 };
	}
	else if (msg == WM_LBUTTONDOWN && selecting) {
		sMu.lock();
		bool s = solving;
		sMu.unlock();
		if (!s) {
			if (!sClicks) {
				start = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
				for (int x = -1; x <= 1; x++) {
					for (int y = -1; y <= 1; y++) {
						img->setPixel(start.x + x, start.y + y, Color{ 0, 255, 0 });
					}
				}
				sClicks++;
			}
			else {
				finish = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
				for (int x = -1; x <= 1; x++) {
					for (int y = -1; y <= 1; y++) {
						img->setPixel(finish.x + x, finish.y + y, Color{ 255, 0, 0 });
					}
				}
				Progressbar * p = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
				p->showComponent();
				sClicks = 0;
				selecting = false;
				std::thread t(calculatePath);
				t.detach();
				currentCursor = CURSOR_WAIT;

			}
			RECT r;
			GetClientRect(hwnd, &r);
			InvalidateRect(hwnd, &r, TRUE);
		}
	}
	if (msg == WM_ERASEBKGND && crop) return TRUE;
	if (msg == WM_PAINT && clicks > 0) {
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		RECT r;
		GetClientRect(hwnd, &r);

		hdc = BeginPaint(hwnd, &paint);

		HDC hdcMem = CreateCompatibleDC(paint.hdc);
		HBITMAP hbmMem = CreateCompatibleBitmap(paint.hdc, r.right - r.left, r.bottom - r.top);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

		FillRect(hdcMem, &r, (HBRUSH)COLOR_WINDOW);
		if (Window::boundWindow != nullptr)
			Window::boundWindow->getCanvas()->draw(hdcMem);
		HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		SelectObject(hdcMem, GetStockBrush(NULL_BRUSH));
		SelectObject(hdcMem, pen);
		Rectangle(hdcMem, min(position_1.x, p.x), min(position_1.y, p.y), max(position_1.x, p.x), max(position_1.y, p.y));
		DeletePen(pen);

		BitBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hbmOld);
		DeleteObject(hbmMem);
		DeleteDC(hdcMem);

		EndPaint(hwnd, &paint);
		return TRUE;
	}
	else if (msg == WM_PAINT && showFinal) {
		hdc = BeginPaint(hwnd, &paint);
		RECT r;
		GetClientRect(hwnd, &r);
		FillRect(hdc, &r, (HBRUSH)COLOR_WINDOW);
		if (Window::boundWindow != nullptr)
			Window::boundWindow->getCanvas()->draw(hdc);
		HPEN pen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
		SelectObject(hdc, pen);
		SelectObject(hdc, GetStockBrush(NULL_BRUSH));
		Rectangle(hdc, min(position_1.x, position_2.x), min(position_1.y, position_2.y), max(position_1.x, position_2.x), max(position_1.y, position_2.y));
		DeletePen(pen);
		EndPaint(hwnd, &paint);
		return TRUE;
	}
	if (msg == WM_MOUSEMOVE && crop) {
		RECT r;
		GetClientRect(display, &r);
		InvalidateRect(display, &r, FALSE);
	}
	return 0;
}
int main(){
//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	display.use();
	WNDCLASSEX update;
	update.lpszMenuName = MAKEINTRESOURCE(ID_MENU);
	update.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON));
	update.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
	display.setWindowProperty(menuName, update);
	display.setWindowProperty(icon, update);
	display.createWindow(300, 300, 800, 800);
	GUI::bindWindow(display);
	Page * p1 = new Page("Page 1", {
		new Scrollbar("testScroll", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 10, 100, scrollHorz, SBS_HORZ),
		new Scrollbar("scrollVert", display.getDimensions().width - 10, 0, 10, display.getDimensions().height, 100, scrollVert, SBS_VERT),
		new Progressbar("progress", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 20, 0, 100)
	});
	p1->setLayout(new AbsoluteLayout());
	mainPage = new MainPage({
		p1
	});
	mainPage->navigateTo("Page 1");
	Progressbar * p = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
	p->setMarquee(true);
	p->hideComponent();
#pragma region eventHandling
	Scrollbar * scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("testScroll");
	scroll->setScrollMessages([](int changePos) {
		for (auto it = display.getCanvas()->getImages()->begin(); it != display.getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX() - changePos, (*it)->getY());
		}
	});
	scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("scrollVert");
	scroll->setScrollMessages(nullptr, [](int changePos) {
		for (auto it = display.getCanvas()->getImages()->begin(); it != display.getCanvas()->getImages()->end(); it++) {
			(*it)->setPosition((*it)->getX(), (*it)->getY() - changePos);
		}
	});
	display.addEventListener(new EventListener([](EventParams ep) { 
		MSG m = Event(WM_SIZE, ep).toMsg();
		mainPage->handleMessage(&m);
	}, WM_SIZE));
	display.addEventListener(new EventListener([](EventParams ep) {
		if (LOWORD(ep.getParam3()) == IDM_LOAD) {
			fileFilter f("Images (.bmp)", { ".bmp" });
			std::string path = openFileDialog(&f);
			if (path.size() > 1) {
				if (img != nullptr) {
					display.deleteImage(img);
//					delete img;
				}
				img = new Image(path.c_str(), 0, 0);
				display.drawImage(img);
				img->toMonochrome();
				POINT origin = { 0, 0 };
				float f = findSkewAngle(img, &origin);
				printf("Angle: %f \n", f);
				rotateImage(img, -f, origin);
				detectSearchBorder(img);
				Scrollbar * scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("testScroll");
				scroll->update(max(img->getWidth() - display.getDimensions().width, 2));
				scroll = (Scrollbar*)mainPage->getCurrentPage()->getControl("scrollVert");
				scroll->update(max(img->getHeight() - display.getDimensions().height, 2));
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_SAVE) {
			std::string path = saveFileDialog();
			if (path.size() > 1) {
				if (img != nullptr)
					img->saveBmp(path.c_str());
			}
		}
		else if (LOWORD(ep.getParam3()) == IDM_CROP) {
			crop = true;
			showFinal = false;
			currentCursor = CURSOR_SELECT;
		}
		else if (LOWORD(ep.getParam3()) == IDM_SOLVE) {
			if (img != nullptr) {
				sMu.lock();
				bool s = solving;
				sMu.unlock();
				if (!s) {
					if (buffer != nullptr) delete buffer;
					buffer = new int[img->getHeight() * img->getWidth()];
					for (int i = 0; i < img->getHeight() * img->getWidth(); i++) {
						int x = i % img->getWidth();
						int y = i / img->getWidth();
						Color c = img->getPixel(x, y);
						buffer[i] = (255 * 255 * 3) - ((int)c.r * c.r + (int)c.g * c.g + (int)c.b * c.b);
					}
					selecting = true;
					currentCursor = CURSOR_DRAW;
				}
			}
		}
	}, WM_COMMAND));
	display.addEventListener(new EventListener([](EventParams ep) {
		MSG m = Event(WM_HSCROLL, ep).toMsg();
		m.hwnd = GUI::useWindow();
		mainPage->handleMessage(&m);
	}, WM_HSCROLL));
	display.addEventListener(new EventListener([](EventParams ep) {
		MSG m = Event(WM_VSCROLL, ep).toMsg();
		m.hwnd = GUI::useWindow();
		mainPage->handleMessage(&m);
	}, WM_VSCROLL));
	display.setWindowProc(customProc);
	cursors[CURSOR_NORMAL] = LoadCursor(NULL, IDC_ARROW);
	cursors[CURSOR_WAIT] = LoadCursor(NULL, IDC_WAIT);
	cursors[CURSOR_DRAW] = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_CURSOR_PEN));
	cursors[CURSOR_SELECT] = LoadCursor(NULL, IDC_CROSS);
	display.addEventListener(new EventListener([](EventParams ep) {
		SetCursor(cursors[currentCursor]);
	}, WM_SETCURSOR));
#pragma endregion 
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) break;
			mainPage->handleMessage(&msg);
		}
		std::lock_guard<std::mutex> lock(iMu);
		if (invalidate) {
			currentCursor = CURSOR_NORMAL;
			Progressbar * pr = (Progressbar*)mainPage->getCurrentPage()->getControl("progress");
			pr->hideComponent();
			RECT r;
			GetClientRect(display, &r);
			InvalidateRect(display, &r, TRUE);
			invalidate = false;
			position_1 = { 0, 0 };
			position_2 = { 0, 0 };
		}
	}
	GUI_CLEANUP(mainPage);
	return 0;
}