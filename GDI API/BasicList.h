#pragma once
#include "Canvas.h"
struct ListCell {
	Image * img;
	Text * txt;
	bool selected = false;
	int x = 0;
	int y = 0;
	COLORREF selectColor = RGB(255, 251, 204);
	ListCell(Text * txt, Image * img, int x = 0, int y = 0) : txt(txt), img(img), x(x), y(y) {};
	~ListCell();
};
#define LIST_BORDER_TRANSPARENT 0
#define LIST_BORDER_SOLID PS_SOLID
#define LIST_BORDER_DASH PS_DASH
#define LIST_BORDER_DOT PS_DOT
#define LIST_BORDER_DASH_DOT PS_DASHDOT
#define LIST_BORDER_DASH_DOTDOT PS_DASHDOTDOT
#define LIST_SPACING_UNEQUAL 0
class BasicList;
typedef void(*BasicListSelectedCallback)(int index, BasicList * parent);
class BasicList : public Drawable{
private:
	std::vector<ListCell *> cells;
	int x;
	int y;
	int width;
	int height;
	COLORREF border;
	int borderStroke;
	int borderStyle;
	int spacing = LIST_SPACING_UNEQUAL;
	int listTop;
	BasicListSelectedCallback select;
public:
	BasicList(int x, int y, int width, int height) : x(x), y(y), width(width), height(height), listTop(0) {};
	~BasicList();
	void addCell(ListCell * cell);
	void draw(HDC hdc);
	void setBorder(COLORREF b, int stroke, int style = LIST_BORDER_SOLID);
	void setSpacing(int spacing) { this->spacing = spacing; }
	void eventHandle(UINT msg, WPARAM w, LPARAM l);
	void selectCallback(BasicListSelectedCallback callback) { select = callback; }
	std::vector<ListCell *> * getCells() { return &cells; }
};