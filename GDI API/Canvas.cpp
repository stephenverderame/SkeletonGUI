#include "Canvas.h"
Canvas::~Canvas()
{
	for (auto it = images.begin(); it != images.end(); it++) {
//		delete *it;
	}
	for (auto it = labels.begin(); it != labels.end(); it++) {
		delete *it;
	}
}

void Canvas::addImage(Image * bmp)
{
	images.push_back(bmp);
}

void Canvas::addText(Text * txt)
{
	labels.push_back(txt);
}

void Canvas::removeText(Text * txt)
{
	labels.erase(std::find(labels.begin(), labels.end(), txt));
}

void Canvas::removeImage(Image * bmp)
{
	images.erase(std::find(images.begin(), images.end(), bmp));
}
void Canvas::addDrawable(Drawable * object)
{
	objects.push_back(object);
}
void Canvas::handleMessages(UINT msg, WPARAM w, LPARAM l)
{
	for (auto o : objects) {
		o->eventHandle(msg, w, l);
	}
}
void Canvas::draw()
{
	PAINTSTRUCT paint;
	BITMAP bm;
	ZeroMemory(&paint, sizeof(paint));
	HDC hdc = BeginPaint(parentWindow, &paint);
	HDC hdcMem = CreateCompatibleDC(hdc);
	draw(hdc);
	DeleteDC(hdcMem);
	EndPaint(parentWindow, &paint);
}

void Canvas::draw(HDC hdc)
{
	BITMAP bm;
	HDC hdcMem = CreateCompatibleDC(hdc);
	for (auto it = images.begin(); it != images.end(); it++) {
		ZeroMemory(&bm, sizeof(bm));
		HBITMAP old = (HBITMAP)SelectObject(hdcMem, (*it)->getHandle());
		GetObject((*it)->getHandle(), sizeof(bm), &bm);
		if ((*it)->updatedSinceLoad()) {
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
			StretchDIBits(hdcMem, 0, 0, (*it)->getForcedWidth(), (*it)->getForcedHeight(), 0, 0, bm.bmWidth, bm.bmHeight, (*it)->getRawData(), &info, NULL, SRCCOPY);
		}
		BitBlt(hdc, (*it)->getX(), (*it)->getY(), bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, old);
	}
	for (Text * t : labels) {
		RECT r;
		bool useFont = false;
		HFONT old = NULL;
		r.left = t->x;
		r.top = t->y;
		r.right = r.left + t->width;
		r.bottom = r.top + t->height;
		if (t->textColor != NULL)
			SetTextColor(hdc, t->textColor);
		if (t->backgroundColor == 0)
			SetBkMode(hdc, TRANSPARENT);
		else {
			SetBkMode(hdc, OPAQUE);
			SetBkColor(hdc, t->backgroundColor);
		}
		if (t->font != NULL)
			useFont = true;
		if (useFont) {
			old = (HFONT)SelectObject(hdc, t->font);
		}
		DrawText(hdc, t->msg.c_str(), t->msg.size(), &r, t->params);
		if (useFont)
			SelectObject(hdc, old);
	}
	for (Drawable * d : objects) {
		d->draw(hdc);
	}
	DeleteDC(hdcMem);
}

