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
#include <guiMainEx.h>
using namespace gui;
MainPage * mainPage;
Image * img;
Window display("Puzzle Solver");
bool crop = false, showFinal = false;
bool pOrigin = false;
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
POINT origin;
HWND ldModelessDialog;
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
float getWndVal(HWND parent, int control) {
	int size = GetWindowTextLength(GetDlgItem(parent, control));
	if (size == 0) return 0;
	char * txt = new char[size];
	float f;
	GetWindowText(GetDlgItem(parent, control), txt, size);
	printf("%s \n", txt);
//	txt[size] = '\0';
	f = atoi(txt);
	delete[] txt;
	return f;
}
BOOL CALLBACK ldDialogProc(HWND hwnd, UINT msg, WPARAM w, LPARAM l) {
	switch (msg) {
	case WM_INITDIALOG:
	{
		HWND autoCheck = GetDlgItem(hwnd, IDC_CHECK1);
		HWND min = GetDlgItem(hwnd, IDC_EDIT1);
		HWND max = GetDlgItem(hwnd, IDC_EDIT2);
		SendMessage(autoCheck, BM_SETCHECK, BST_CHECKED, 1);
		SetWindowText(min, "0");
		SetWindowText(max, "360");
		EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), 0);
		break;
	}
	case WM_COMMAND:
	{
		if (LOWORD(w) == IDC_BUTTON1) {
			float * data = new float[2];
			float min, max;
			if (SendMessage(GetDlgItem(hwnd, IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED) {
				int size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT1));
				char * entryText = new char[size + 1];
				GetWindowText(GetDlgItem(hwnd, IDC_EDIT1), entryText, size);
				entryText[size] = '\0';
				for (int i = 0; i < size; i++) {
					if ((!isdigit(entryText[i])) && entryText[i] != '.') {
						min = 0;
						break;
					}
				}
				min = atof(entryText);
				delete[] entryText;
				size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT2));
				entryText = new char[size + 1];
				GetWindowText(GetDlgItem(hwnd, IDC_EDIT2), entryText, size);
				entryText[size] = '\0';
				for (int i = 0; i < size; i++) {
					if ((!isdigit(entryText[i])) && entryText[i] != '.') {
						max = 360;
						break;
					}
				}
				max = atof(entryText);
				delete[] entryText;

			}
			else {
				int size = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT3));
				char * entryText = new char[size + 1];
				GetWindowText(GetDlgItem(hwnd, IDC_EDIT3), entryText, size);
				entryText[size] = '\0';
				for (int i = 0; i < size; i++) {
					if ((!isdigit(entryText[i])) && entryText[i] != '.') {
						min = 0;
						max = 360;
						break;
					}
				}
				min = atof(entryText);
				max = min;
				delete[] entryText;

			}
			data[0] = min;
			data[1] = max;
			Event e(WM_COMMAND, EventParams((LPARAM)data, w));
			display.fireEvent(e);
//			EndDialog(hwnd, IDC_BUTTON1);
			DestroyWindow(hwnd);
		}
		else if (HIWORD(w) == EN_CHANGE) {
			int control;
			COLORREF c;
			if ((HWND)l == GetDlgItem(hwnd, IDC_EDIT1)) {
				control = IDC_EDIT1;
				c = RGB(128, 0, 128);
			}
			else if ((HWND)l == GetDlgItem(hwnd, IDC_EDIT2)) {
				control = IDC_EDIT2;
				c = RGB(255, 255, 0);
			}
			else {
				control = IDC_EDIT3;
				c = RGB(255, 0, 0);
			}
			float theta = getWndVal(hwnd, control);
			printf("Theta: %f \n", theta);
			std::vector<POINT> line;
			for (int i = origin.x; i < img->getWidth(); i++) {
				line.push_back({ i, origin.y });
			}
			float rotMat[] = {
				cos(radians(theta)),	-sin(radians(theta)),
				sin(radians(theta)),	 cos(radians(theta))
			};
			for (int i = 0; i < line.size(); i++) {
				line[i] = matrixMultiply(rotMat, line[i]);
			}
			PAINTSTRUCT p;
			HDC dc = BeginPaint(display, &p);
			for (int i = 0; i < line.size(); i++) {
				SetPixel(dc, line[i].x, line[i].y, c);
			}
			EndPaint(display, &p);
		}
		else if (HIWORD(w) == BN_CLICKED) {
			if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK1)) {
				BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
				EnableWindow(GetDlgItem(hwnd, IDC_EDIT3), !enable);
			}
			else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK3)) {
				BOOL enable = SendMessage((HWND)l, BM_GETCHECK, 0, 0);
				for (int i = IDC_CHECK1; i < IDC_CHECK3; i++) {
					if (i == IDC_BUTTON1) continue;
					EnableWindow(GetDlgItem(hwnd, i), !enable);
				}
			}
			else if ((HWND)l == GetDlgItem(hwnd, IDC_CHECK2)) {
				if (SendMessage((HWND)l, BM_GETCHECK, 0, 0)) {
					pOrigin = true;
					currentCursor = CURSOR_SELECT;
					MessageBox(hwnd, "Click the point on the image where you want the center of rotation", "Pick Origin", MB_OK | MB_ICONINFORMATION);
				}
				else {
					currentCursor = CURSOR_NORMAL;
					pOrigin = false;
				}
			}
		}
		break;
	}
	default:
		return FALSE; //not handled
	return TRUE;
	}
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
	else if (msg == WM_LBUTTONDOWN && pOrigin) {
		origin = { GET_X_LPARAM(l), GET_Y_LPARAM(l) };
		HDC dc = BeginPaint(hwnd, &paint);
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				SetPixel(dc, x, y, RGB(0, 191, 255));
			}
		}
		EndPaint(hwnd, &paint);
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
	int sections[] = { 100, -1 };
	Page * p1 = new Page("Page 1", {
		new Scrollbar("testScroll", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 10, 100, scrollHorz, SBS_HORZ),
		new Scrollbar("scrollVert", display.getDimensions().width - 10, 0, 10, display.getDimensions().height, 100, scrollVert, SBS_VERT),
		new Progressbar("progress", 10, display.getDimensions().height - 20, display.getDimensions().width - 20, 20, 0, 100),
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
		if (LOWORD(ep.getParam3()) == IDM_LOAD_SEARCH) {
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
//				int ret = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOAD), display, ldDialogProc);
				ldModelessDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOAD), display, ldDialogProc);
				ShowWindow(ldModelessDialog, SW_SHOW);
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
		else if (LOWORD(ep.getParam3()) == IDM_SOLVE_MAZE) {
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
		else if (LOWORD(ep.getParam3()) == IDC_BUTTON1) {
			float * bounds = (float*)ep.getParaml();
			Bounds b{ bounds[0], bounds[1] };
			float f = findSkewAngle(img, &origin, &b);
			delete[] bounds;
			printf("Angle: %f \n", f);
			//				rotateImage(img, -f, origin);
			auto list = getCharacterLocations(img);
			//				augmentDataSet(list, {'T', 'N', 'E', 'R', 'R', 'U', 'C', 'M', 'M', 'A', 'G', 'N', 'E', 'T', 'I', 'S', 'M', 'P'}, img);
			//				augmentDataSet(list, { 'D', 'N', 'T', 'I', 'G', 'C', 'A', 'J', 'C', 'A', 'O', 'J', 'O', 'H', 'S', 'I', 'E', 'I', 'B', 'A' }, img);
			//				augmentDataSet(list, { 'H', 'M', 'E', 'G', 'Q', 'T', 'Z', 'M', 'R', 'H', 'Y', 'L', 'S', 'R' }, img);
			identifyLetters(img, list);
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