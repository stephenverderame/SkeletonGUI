#include "BasicList.h"
BasicList::~BasicList()
{
	for (int i = 0; i < cells.size(); i++)
		delete cells[i];
}

void BasicList::addCell(ListCell * cell)
{
	cell->x = x;
	cell->y = y + (cells.size() * spacing);
	cells.push_back(cell);
}

void BasicList::draw(HDC hdc)
{
	int i = 1;
	for (ListCell * cell : cells) {
		if (cell->y + spacing - listTop > y + height || cell->y - listTop < y) continue;
		bool useFont = false;
		HFONT old = NULL;
		if (cell->selected) {
			HBRUSH br = CreateSolidBrush(cell->selectColor);
			HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, br);
			HPEN oldPen = (HPEN)SelectObject(hdc, (HPEN)GetStockObject(NULL_PEN));
			Rectangle(hdc, cell->x, cell->y - listTop, cell->x + width, cell->y + spacing - listTop);
			SelectObject(hdc, oldBrush);
			SelectObject(hdc, oldPen);
			DeleteObject(br);
		}
		RECT r;
		r.left = cell->x + cell->txt->x;
		r.top = cell->y - listTop + cell->txt->y;
		r.right = r.left + cell->txt->width;
		r.bottom = r.top + cell->txt->height;
		if (cell->txt->textColor != NULL)
			SetTextColor(hdc, cell->txt->textColor);
		if (cell->txt->backgroundColor == 0)
			SetBkMode(hdc, TRANSPARENT);
		else {
			SetBkMode(hdc, OPAQUE);
			SetBkColor(hdc, cell->txt->backgroundColor);
		}
		if (cell->txt->font != NULL)
			useFont = true;
		if (useFont) {
			old = (HFONT)SelectObject(hdc, cell->txt->font);
		}
		DrawText(hdc, cell->txt->msg.c_str(), cell->txt->msg.size(), &r, cell->txt->params);
		if (useFont)
			SelectObject(hdc, old);
		//image
		if (cell->img != nullptr) {
			HDC hdcMem = CreateCompatibleDC(hdc);
			BITMAP bm;
			ZeroMemory(&bm, sizeof(bm));
			HBITMAP old = (HBITMAP)SelectObject(hdcMem, cell->img->getHandle());
			GetObject(cell->img->getHandle(), sizeof(bm), &bm);
			if (cell->img->updatedSinceLoad()) {
				BITMAPINFOHEADER header;
				header.biBitCount = 24;
				header.biClrImportant = 0;
				header.biClrUsed = 0;
				header.biCompression = 0;
				header.biHeight = bm.bmHeight;
				header.biWidth = bm.bmWidth;
				header.biSize = sizeof(BITMAPINFOHEADER);
				header.biSizeImage = bm.bmWidthBytes * bm.bmHeight;
				header.biXPelsPerMeter = 2800;
				header.biYPelsPerMeter = 2800;
				BITMAPINFO info;
				ZeroMemory(&info, sizeof(info));
				info.bmiHeader = header;
				StretchDIBits(hdcMem, 0, 0, cell->img->getForcedWidth(), cell->img->getForcedHeight(), 0, 0, bm.bmWidth, bm.bmHeight, cell->img->getRawData(), &info, NULL, SRCCOPY);
			}
			BitBlt(hdc, cell->x + cell->img->getX(), cell->y - listTop + cell->img->getY(), bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, old);
		}
		//divider
		HPEN lPen = CreatePen(PS_SOLID, 2, RGB(200, 200, 200));
		HPEN oldPen = (HPEN)SelectObject(hdc, lPen);
		MoveToEx(hdc, x + 10, cell->y + spacing - listTop, NULL);
		LineTo(hdc, x + width - 10, cell->y + spacing - listTop);
		SelectObject(hdc, oldPen);
		DeleteObject(lPen);
	}
	if (border != LIST_BORDER_TRANSPARENT) {
		HPEN pen = CreatePen(borderStyle, borderStroke, border);
		HPEN old = (HPEN)SelectObject(hdc, pen);
		HBRUSH oldB = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, x, y, x + width, y + height);
		SelectObject(hdc, old);
		SelectObject(hdc, oldB);
		DeleteObject(pen);
	}
}

void BasicList::setBorder(COLORREF b, int stroke, int style)
{
	border = b;
	borderStroke = stroke;
	borderStyle = style;
}

void BasicList::eventHandle(UINT msg, WPARAM w, LPARAM l)
{
	if (msg == WM_LBUTTONDBLCLK) {
		int xClick = LOWORD(l);
		if (xClick < x || xClick > x + width) return;
		int yClick = HIWORD(l);
		for (int i = 0; i < cells.size(); i++) {
			if (yClick >= cells[i]->y - listTop && yClick <= cells[i]->y + spacing - listTop)
				if (this->select != nullptr) this->select(i, this);
		}
	}
	if (msg == WM_LBUTTONDOWN) {
		int xClick = LOWORD(l);
		if (xClick < x || xClick > x + width) return;
		int yClick = HIWORD(l);
		for (auto c : cells) {
			if (yClick >= c->y - listTop && yClick <= c->y + spacing - listTop)
				c->selected = true;
			else
				c->selected = false;
		}
		RECT r;
		GetClientRect(gui::GUI::useWindow(), &r);
		InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	}
	if (msg == WM_MOUSEWHEEL) {
		int old = listTop;
		listTop += (GET_WHEEL_DELTA_WPARAM(w) / (float)WHEEL_DELTA) * spacing;
		if (listTop < 0) listTop = 0;
		if (listTop > cells.size() * spacing - height) listTop = old;
		if (listTop != old) {
			RECT r;
			GetClientRect(gui::GUI::useWindow(), &r);
			InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
		}
	}
	else if (msg == WM_KEYDOWN) {
		int old = listTop;
		if (w == VK_DOWN)
			listTop -= spacing;
		else if (w == VK_UP)
			listTop += spacing;
		else return;
		if (listTop < 0) listTop = 0;
		if (listTop > cells.size() * spacing - height) listTop = old;
		if (listTop != old) {
			RECT r;
			GetClientRect(gui::GUI::useWindow(), &r);
			InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
		}
	}
}

ListCell::~ListCell()
{
	if (img != nullptr)
		delete img;
	if (txt != nullptr)
		delete txt;
}
