#pragma once
#pragma once
#ifdef GUI_VISUAL_STYLES_6
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#include <Windows.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <initializer_list>
#include "Layout.h"
#include "GUIS.h"
namespace gui {
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#define GUI_PARAM_AND |
#define GUI_PARAM_EXCLUDE ^
#ifndef GUI_MACRO_DESC_ONLY
#define and GUI_PARAM_AND
#define exclude GUI_PARAM_EXCLUDE
#endif //GUI_MACRO_DESC_ONLY
	class Progress : public IBindStatusCallback {
	public:
		ULONG progress;
		ULONG progressMax;
	public:
		Progress() { progress = 0; progressMax = 0; };
		~Progress() {};
		STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText) {
			progress = ulProgress;
			progressMax = ulProgressMax;
			return S_OK;
		}
		STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR *pformatetc, STGMEDIUM __RPC_FAR *pstgmed)
		{
			return E_NOTIMPL;
		}
		STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding __RPC_FAR *pib)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetPriority)(LONG __RPC_FAR *pnPriority)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnLowResource)(DWORD reserved)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError)
		{
			return E_NOTIMPL;
		}

		STDMETHOD(GetBindInfo)(DWORD __RPC_FAR *grfBINDF, BINDINFO __RPC_FAR *pbindinfo)
		{
			return E_NOTIMPL;
		}


		STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR *punk)
		{
			return E_NOTIMPL;
		}

		// IUnknown stuff
		STDMETHOD_(ULONG, AddRef)()
		{
			return 0;
		}

		STDMETHOD_(ULONG, Release)()
		{
			return 0;
		}

		STDMETHOD(QueryInterface)(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject)
		{
			return E_NOTIMPL;
		}
	};
	enum ControlType {
		scrollVert,
		scrollHorz,
	};
	class Control {
	protected:
		HWND handle;
		HWND parent;
		bool selfHandle;
		std::string name;
		Dimensions dimensions;
		Dimensions originalDims;
		ControlType controlType;
	protected:
		inline void setStyle(int params) {
			SetWindowLongPtr(handle, GWL_STYLE, params);
		}
	public:
		Control() : selfHandle(false){}
		~Control() { DestroyWindow(handle); }
		inline virtual void hideComponent() {
			ShowWindow(handle, SW_HIDE);
		}
		inline virtual void showComponent() {
			ShowWindow(handle, SW_SHOW);
		}
		inline virtual void disableControl() {
			EnableWindow(handle, FALSE);
		}
		inline virtual void enableControl() {
			EnableWindow(handle, TRUE);
		}
		inline virtual bool msgFromControl(MSG * msg) {
			return msg->hwnd == handle;
		}
		void dpiScale() {
			int iDpi;
			HDC dc = GetDC(NULL);
			iDpi = GetDeviceCaps(dc, LOGPIXELSX);
			MessageBox(NULL, std::to_string(iDpi).c_str(), "", MB_OK);
			ReleaseDC(NULL, dc);
			int dpiScaledX = MulDiv(dimensions.x, iDpi, 96);
			int dpiScaledY = MulDiv(dimensions.y, iDpi, 96);
			int dpiScaledWidth = MulDiv(dimensions.width, iDpi, 96);
			int dpiScaledHeight = MulDiv(dimensions.height, iDpi, 96);
			SetWindowPos(handle, handle, dpiScaledX, dpiScaledY, dpiScaledWidth, dpiScaledHeight, SWP_NOZORDER | SWP_NOACTIVATE);
			dimensions = { dpiScaledX, dpiScaledY, dpiScaledWidth, dpiScaledHeight };
		}
		inline void setPos(int x, int y, int width, int height) {
			MoveWindow(handle, x, y, width, height, TRUE);
			dimensions = { x, y, width, height };
		}
		inline bool doesDefaultMsgHandle() { return selfHandle; }
		virtual void handleMsg(MSG * msg) {};
		inline void addStyle(int params) {
			SetWindowLongPtr(handle, GWL_STYLE, GetWindowLongPtr(handle, GWL_STYLE) and params);
		}
		inline Dimensions getDimensions() { return dimensions; }
		inline Dimensions getOriginalDimensions() { return originalDims; }
		inline void setOriginalDimensions(Dimensions dims) { originalDims = dims; }
		inline HWND getHandle() { return handle; }
		inline std::string getName() { return name; }
		ControlType * getControlType() { return &controlType; }
	};
#define GUI_BUTTON_LIGHT_BORDER (BS_DEFPUSHBUTTON)
	class Button : public Control {
	private:
		void(*onClick)();
	protected:
		void init() {
			handle = CreateWindow("BUTTON", "", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 0, 0, 100, 50, parent, NULL, NULL, NULL);
			selfHandle = false;
		}
	public:
		Button(char * id, int x, int y, int width, int height, char * title, void(*onClick)() = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			parent = window;
			name = id; dimensions = { x, y, width, height }; originalDims = dimensions;
			if (onClick != NULL) {
				selfHandle = true;
				this->onClick = onClick;
			}
			if (params & GUI_BUTTON_LIGHT_BORDER) {
				handle = CreateWindow("BUTTON", title, WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			}
			else
				handle = CreateWindow("BUTTON", title, WS_CHILD | BS_DEFPUSHBUTTON | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
		}
		Button() {
			init();
		}
		inline void setClick(void(*click)()) {
			selfHandle = true;
			onClick = click;
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd != handle || msg->message != WM_LBUTTONDOWN) return;
			Sleep(100);
			onClick();
		}
	};
#define GUI_TEXTFIELD_VERT_SCROLL (ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL)
#define GUI_TEXTFIELD_HOR_SCROLL (ES_AUTOHSCROLL | WS_HSCROLL)
#define GUI_TEXTFIELD_MULTILINE (ES_MULTILINE)
#define GUI_TEXTFIELD_PASSFIELD (ES_PASSWORD)
	class TextField : public Control {
	protected:
		void init() {
			handle = CreateWindow("EDIT", "", WS_CHILD, 0, 0, 100, 50, parent, NULL, NULL, NULL);
		}
	public:
		TextField(char * id, int x, int y, int width, int height, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = { x, y, width, height }; originalDims = dimensions;
			parent = window;
			handle = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
		}
		TextField(char * id, int x, int y, int width, int height, char * text, bool typing = true, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = { x, y, width, height }; originalDims = dimensions;
			parent = window;
			if (!typing) selfHandle = true;
			handle = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			Edit_SetText(handle, text);
		}
		TextField() {
			init();
		}
		void disableTyping() {
			selfHandle = true;
		}
		void enableTyping() {
			selfHandle = false;
		}
		void handleMsg(MSG * msg) {
			if (msg->message == WM_LBUTTONDOWN && msg->hwnd == handle)
				SetFocus(parent);

		}
		void setPasswordChar(char echo) {
			SendMessage(handle, EM_SETPASSWORDCHAR, echo, NULL);
		}
		void setText(char * text) {
			SetWindowText(handle, text);
		}
		void addText(char * text) {
			SendMessage(handle, EM_SETSEL, 0, -1);
			SendMessage(handle, EM_SETSEL, -1, -1);
			SendMessage(handle, EM_REPLACESEL, 0, (LPARAM)text);
		}
		std::string getText() {
			size_t size = GetWindowTextLength(handle);
			char * text = new char[size + 1];
			GetWindowText(handle, text, size + 1);
			std::string ret(text);
			delete[] text;
			return ret;
		}
	};
	class Label : public Control {
	protected:
		void init() {
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 0, 100, 50, parent, NULL, NULL, NULL);
			SetWindowText(handle, "Label:");
		}
	public:
		Label(char * id, int x, int y, char * text, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, (int)strlen(text) * 10, 20 }; originalDims = dimensions;
			parent = window;
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | params, x, y, strlen(text) * 10, 20, parent, NULL, NULL, NULL);
			SetWindowText(handle, text);
		}
		Label(char * id, int x, int y, int width, int height, char * text, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, width, height }; originalDims = dimensions;
			parent = window;
			handle = CreateWindow("STATIC", "ST_U", WS_CHILD | WS_VISIBLE | params, x, y, width, height, parent, NULL, NULL, NULL);
			SetWindowText(handle, text);
		}
		Label() {
			init();
		}
		void setText(char * text) {
			SetWindowText(handle, text);
		}

	};
	class Radiobutton : public Control {
	private:
		std::vector<HWND> handles;
		int lastY;
		int X;
	public:
		Radiobutton(char * id, int x, int y, char * text, int params = NULL, HWND window = GUI::useWindow()) : lastY(y), X(x) {
			name = id;
			selfHandle = true;
			parent = window;
			handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, x, y, (strlen(text) * 10) + 30, 20, window, NULL, NULL, NULL);
			handles.push_back(handle);
			setCheck(0, true);
		}
		~Radiobutton() {
			for (int i = 0; i < handles.size(); i++) {
				DestroyWindow(handles[i]);
			}
		}
		void addChoice(char * text, int params = NULL) {
			lastY += 30;
			HWND handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, X, lastY, (strlen(text) * 10) + 30, 20, parent, NULL, NULL, NULL);
			handles.push_back(handle);
		}
		Radiobutton(char * id, int x, int y, std::initializer_list<char*> choices, int params = NULL, HWND window = GUI::useWindow()) : lastY(y), X(x) {
			name = id;
			std::vector<char*> titles;
			for (char * text : choices) {
				titles.push_back(text);
			}
			selfHandle = true;
			parent = window;
			handle = CreateWindow("BUTTON", titles[0], WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON | params, x, y, (strlen(titles[0]) * 10) + 30, 20, window, NULL, NULL, NULL);
			handles.push_back(handle);
			for (int i = 1; i < titles.size(); i++) {
				addChoice(titles[i], params);
			}
		}
		void handleMsg(MSG * msg) {
			bool cont = false;
			int caller = 0;
			for (int i = 0; i < handles.size(); i++) {
				if (msg->hwnd == handles[i]) {
					cont = true;
					caller = i;
				}
			}
			if (!cont) return;
			if (msg->message == WM_LBUTTONDOWN) {
				if (SendMessage(msg->hwnd, BM_GETCHECK, 0, 0) == BST_UNCHECKED) {
					SendMessage(msg->hwnd, BM_SETCHECK, BST_CHECKED, 1);
					for (int i = 0; i < handles.size(); i++) {
						if (i == caller) continue;
						SendMessage(handles[i], BM_SETCHECK, BST_UNCHECKED, 1);
					}
				}
			}
		}
		bool msgFromControl(MSG * msg) {
			for (int i = 0; i < handles.size(); i++) {
				if (msg->hwnd == handles[i]) return true;
			}
			return false;
		}
		void hideComponent() {
			for (int i = 0; i < handles.size(); i++) {
				ShowWindow(handles[i], SW_HIDE);
			}
		}
		void showComponent() {
			for (int i = 0; i < handles.size(); i++) {
				ShowWindow(handles[i], SW_SHOW);
			}
		}
		int getChecked() {
			for (int i = 0; i < handles.size(); i++) {
				if (SendMessage(handles[i], BM_GETCHECK, 0, 0) == BST_CHECKED)
					return i;
			}
		}
		void setCheck(int id, bool isChecked) {
			if (isChecked) {
				SendMessage(handles[id], BM_SETCHECK, BST_CHECKED, 1);
			}
			else {
				SendMessage(handles[id], BM_SETCHECK, BST_UNCHECKED, 1);
			}
		}
	};
	class Checkbox : public Control {
	private:
		void(*onClick)();
	public:
		Checkbox(char * id, int x, int y, char * text, void(*click)() = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, 30 + ((int)strlen(text) * 10), 20 }; originalDims = dimensions;
			parent = window;
			handle = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | params, x, y, 30 + (strlen(text) * 10), 20, window, NULL, NULL, NULL);
			if (click != NULL) {
				selfHandle = true;
				onClick = click;
			}
		}
		bool isChecked() {
			return SendMessage(handle, BM_GETCHECK, 0, 0) == BST_CHECKED;
		}
		void setCheck(bool check) {
			if (check) SendMessage(handle, BM_SETCHECK, BST_CHECKED, 1);
			else SendMessage(handle, BM_SETCHECK, BST_UNCHECKED, 1);
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd != handle || msg->message != WM_LBUTTONDOWN) return;
			if (onClick != NULL) onClick();
		}
	};
	class Combobox : public Control {
	private:
		void(*onSelect)(std::string, int);
		std::string selected;
		int selectedIndex;
	public:
		Combobox(char * id, int x, int y, int width, int height, void(*select)(std::string, int) = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = { x, y, width, height }; originalDims = dimensions;
			parent = window;
			selfHandle = true;
			onSelect = select;
			handle = CreateWindow("COMBOBOX", "Test", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | params, x, y, width, height, window, NULL, NULL, NULL);
		}
		inline void addChoice(char * choice) {
			SendMessage(handle, CB_ADDSTRING, 0, (LPARAM)choice);
		}
		Combobox(char * id, int x, int y, std::initializer_list<char*> list, int width, int defaultSelection = 0,
			void(*select)(std::string, int) = NULL, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, width, (int)list.size() * 50 }; originalDims = dimensions;
			parent = window;
			onSelect = select;
			handle = CreateWindow("COMBOBOX", "Test", CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | params, x, y, width, list.size() * 50, window, NULL, NULL, NULL);
			std::vector<char*> items;
			for (char * item : list) {
				items.push_back(item);
				addChoice(item);
			}
			SendMessage(handle, CB_SETCURSEL, defaultSelection, 0);
		}
		inline void selectChoice(int choice) {
			SendMessage(handle, CB_SETCURSEL, choice, 0);
		}
		int getSelected(std::string & selectedName) {
			int index = SendMessage(handle, CB_GETCURSEL, 0, 0);
			char listItem[256];
			SendMessage(handle, CB_GETLBTEXT, index, (LPARAM)listItem);
			selectedName = listItem;
			return index;
		}
		void handleMsg(MSG * msg) {
			if (msg->hwnd == handle && msg->message == WM_COMMAND && HIWORD(msg->wParam) == CBN_SELCHANGE) {
				std::string name;
				int index = getSelected(name);
				onSelect(name, index);
			}
		}
	};
	class Slider : public Control {
	public:
		Slider(char * id, int x, int y, int width, int height, int g_min, int g_max, int params = NULL, HWND window = GUI::useWindow()) {
			name = id; dimensions = { x, y, width, height }; originalDims = dimensions;
			handle = CreateWindow(TRACKBAR_CLASS, "", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | params, x, y, width, height, window, NULL, NULL, NULL);
			SendMessage(handle, TBM_SETRANGE, TRUE, MAKELONG(g_min, g_max));
			SendMessage(handle, TBM_SETPOS, TRUE, g_max);

		}
		inline void setPos(int pos) {
			SendMessage(handle, TBM_SETPOS, TRUE, pos);
		}
		inline int getPos() {
			return SendMessage(handle, TBM_GETPOS, 0, 0);
		}
		inline void setRange(int g_min, int g_max) {
			SendMessage(handle, TBM_SETRANGE, TRUE, MAKELONG(g_min, g_max));
		}
		inline void setStep(int interval) {
			SendMessage(handle, TBM_SETTICFREQ, interval, 0);
		}
	};
#define GUI_PROGRESSBAR_MARQUEE (PBS_MARQUEE)
	class Progressbar : public Control {
	public:
		Progressbar(char * id, int x, int y, int width, int height, int g_min, int g_max, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, width, height }; originalDims = dimensions;
			handle = CreateWindow(PROGRESS_CLASS, "", WS_CHILD | WS_VISIBLE | params, x, y, width, height, window, NULL, NULL, NULL);
			if (!(params & GUI_PROGRESSBAR_MARQUEE))
				SendMessage(handle, PBM_SETRANGE, NULL, MAKELONG(g_min, g_max));
		}
		inline void setPos(int pos) {
			SendMessage(handle, PBM_SETPOS, pos, NULL);
		}
		inline int getPos() {
			return SendMessage(handle, PBM_GETPOS, NULL, NULL);
		}
		inline void setRange(int low, int high) {
			SendMessage(handle, PBM_SETRANGE, 0, MAKELONG(low, high));
		}
		inline void setMarquee(bool enable) {
			if (enable) {
				this->addStyle(PBS_MARQUEE);
				SendMessage(handle, PBM_SETMARQUEE, TRUE, 30);
			}
			else {
				this->setStyle(WS_CHILD and WS_VISIBLE);
				SendMessage(handle, PBM_SETMARQUEE, FALSE, 30);
			}
		}
	};
#define SCROLLBAR_KEEP_PROP -1
	class Scrollbar : public Control {
	private:
		int currentPos;
		RECT wndDim;
		SCROLLINFO info;
	private:
		std::function<void(int)> hScrollAction;
		std::function<void(int)> vScrollAction;
	private:
		void setScrollInfo(ControlType * c = nullptr) {
			if (c != nullptr) {
				SetScrollInfo(parent, *c == scrollHorz ? SB_HORZ : SB_VERT, &info, TRUE);
				return;
			}
			switch (controlType) {
			case scrollHorz:
				SetScrollInfo(parent, SB_HORZ, &info, TRUE);
				break;
			case scrollVert:
				SetScrollInfo(parent, SB_VERT, &info, TRUE);
				break;
			}
		}
	public:
		Scrollbar(char * id, int x, int y, int width, int height, int max, ControlType type, int params = NULL, HWND window = GUI::useWindow()) {
			name = id;
			dimensions = { x, y, width, height }; originalDims = dimensions;
			selfHandle = true;
			parent = window;
//			handle = CreateWindow("SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE | params, x, y, width, height, window, NULL, NULL, NULL);
			this->controlType = type;
			currentPos = 0;
			GetClientRect(window, &wndDim);
			info.cbSize = sizeof(info);
			info.fMask = SIF_ALL;
			info.nMin = 0;
			info.nMax = max;
			info.nPos = 0;
			info.nPage = 1;
			this->setScrollInfo();
		}
		void setScrollMessages(std::function<void(int)> hScroll = nullptr, std::function<void(int)> vScroll = nullptr) {
			if (hScroll != nullptr) hScrollAction = hScroll;
			if (vScroll != nullptr) vScrollAction = vScroll;
		}
		void update(int max = -1, int page = -1, int min = -1, int pos = -1) {
			info.fMask = 0;
			if (max != SCROLLBAR_KEEP_PROP) {
				info.nMax = max;
				info.fMask |= SIF_RANGE;
			}
			if (page != SCROLLBAR_KEEP_PROP) {
				info.nPage = page;
				info.fMask |= SIF_PAGE;
			}
			if (min != SCROLLBAR_KEEP_PROP) {
				info.nMin = min;
				info.fMask |= SIF_RANGE;
			}
			if (pos != SCROLLBAR_KEEP_PROP) {
				info.nPos = pos;
				info.fMask |= SIF_POS;
			}
			SetScrollInfo(parent, controlType == scrollHorz ? SB_HORZ : SB_VERT, &info, TRUE);
		}
		void handleMsg(MSG * msg) {
//			if (msg->hwnd != parent) return;
			if (msg->message == WM_SIZE) {
				info.cbSize = sizeof(info);
				info.fMask = SIF_RANGE;
				info.nMax = controlType == scrollHorz ? LOWORD(msg->lParam) - GUI::getDimensions().first : HIWORD(msg->lParam) - GUI::getDimensions().second;
				SetScrollInfo(parent, controlType == scrollHorz ? SB_HORZ : SB_VERT, &info, TRUE);
			}
			if (msg->message == WM_HSCROLL  || msg->message == WM_VSCROLL) {
				if ((msg->message == WM_HSCROLL && controlType == scrollVert) || (msg->message == WM_VSCROLL && controlType == scrollHorz)) return;
				GetScrollInfo(parent, msg->message == WM_HSCROLL ? SB_HORZ : SB_VERT, &info);
				int newPos;
				switch (LOWORD(msg->wParam)) {
				case SB_THUMBPOSITION:
					newPos = HIWORD(msg->wParam);
					break;
				case SB_LINEUP:
					newPos = info.nPos - info.nPage;
					break;
				case SB_LINEDOWN:
					newPos = info.nPos + info.nPage;
					break;
				case SB_PAGEUP:
					newPos = info.nPos - info.nPage;
					break;
				case SB_PAGEDOWN:
					newPos = info.nPos + info.nPage;
					break;
				default:
					newPos = info.nPos;
					break;
				}
				newPos = max(0, newPos);
				newPos = min(info.nMax, newPos);
				if (newPos == info.nPos) return;
				int changePos = newPos - info.nPos;
				info.nPos = newPos;
				if (hScrollAction != nullptr && msg->message == WM_HSCROLL) hScrollAction(changePos * info.nPage);
				if (vScrollAction != nullptr && msg->message == WM_VSCROLL) vScrollAction(changePos * info.nPage);
				ScrollWindow(parent, msg->message == WM_HSCROLL ? -changePos * info.nPage : 0, msg->message == WM_HSCROLL ? 0 : -changePos * info.nPage, NULL, NULL);
				UpdateWindow(parent);
				info.cbSize = sizeof(info);
				info.fMask = SIF_POS;
				SetScrollInfo(parent, msg->message == WM_HSCROLL ? SB_HORZ : SB_VERT, &info, TRUE);
			}
		}
	};
#define GUI_CLEANUP(MAINPAGE) (delete (MAINPAGE))
}