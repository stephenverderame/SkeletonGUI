#pragma once
#include "window.h"
#include <string>
#define round(a) ((a) - (int)(a) >= 0.5 ? (int)(a) + 1 : (int)(a))
#define max(a, b) ((a) > (b) ? (a) : (b))
//static Dimensions wndDimensions;
class Layout {
public:
	virtual Dimensions resize(LPARAM size, Dimensions dim) = 0; //Loword of LPARAM of WM_SIZE is width
	virtual Dimensions addControl(Dimensions dim) = 0; //dimensions given will be proportion of window out of 10,000
};
enum absLayoutProperties {
	abs_posProportional = 1,
	abs_sizeProportional = 2,
	abs_posAbsolute = 4,
	abs_sizeAbsolute = 8
};
enum stackLayoutProperties {
	st_left = 1,
	st_center = 2,
	st_right = 4,
	st_top = 8,
	st_middle = 16,
	st_bottom = 32
};
class AbsoluteLayout : public Layout {
private:
	int props;
public:
	AbsoluteLayout(absLayoutProperties p) : props(p) {}
	AbsoluteLayout() : props(abs_posProportional | abs_sizeProportional) {};
	Dimensions resize(LPARAM size, Dimensions originaldim);
	Dimensions addControl(Dimensions dim);
};
class StackLayout : public Layout {
private:
	int props;
	int maxWidth;
	int maxHeight;
	int stackPointer = 0;
	int spacing = 10;
protected:
	static BOOL CALLBACK enumChildProc(HWND hwnd, LPARAM lp);
	static BOOL CALLBACK enumMax(HWND hwnd, LPARAM lp);
public:
	StackLayout() : props(st_center), maxWidth(INT_MIN), maxHeight(INT_MIN) {};
	StackLayout(stackLayoutProperties p) : props(p), maxWidth(INT_MIN), maxHeight(INT_MIN) {};
	StackLayout(stackLayoutProperties p, HWND window);
	Dimensions resize(LPARAM size, Dimensions dim);
	Dimensions addControl(Dimensions dim);
	void formatCurrentControls(HWND window);
	void setPadding(int i) { spacing = i; }

};