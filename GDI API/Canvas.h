#pragma once
#include "GUIS.h"
#include "Drawable.h"
#include <vector>
typedef unsigned char channel;
struct Color {
	channel r;
	channel g;
	channel b;
	long colorBufferDataWidth;
	long colorBufferDataHeight;
	long colorBufferDataScanline;
	int avg() { return ((int)r + g + b) / 3; }
};
enum txtParams {
	textNone = NULL,
	textBottom = DT_BOTTOM,
	textCalcRect = DT_CALCRECT,
	textCenter = DT_CENTER,
	textLeft = DT_LEFT,
	textRight = DT_RIGHT,
	textTop = DT_TOP,
	textDotDotDot = DT_END_ELLIPSIS,
	textNoclip = DT_NOCLIP,
	textSingleline = DT_SINGLELINE,
	textWordBreak = DT_WORDBREAK,
	textWordDotDotDot = DT_WORD_ELLIPSIS
};
#define TEXT_BACKGROUND_COLOR_TRANSPARENT 0
struct Text {
	std::string msg;
	int x;
	int y;
	int width;
	int height;
	COLORREF textColor = NULL;
	COLORREF backgroundColor = TEXT_BACKGROUND_COLOR_TRANSPARENT;
	HFONT font = NULL;
	txtParams params = (txtParams)0;
	Text(int x, int y, int width, std::string msg, int height = 20, txtParams params = textNone, COLORREF textColor = 0, COLORREF backgroundColor = 0, HFONT font = 0) : x(x), y(y), 
		width(width), height(height), msg(msg), params(params), backgroundColor(backgroundColor), textColor(textColor), font(font) {};
};
class Image {
private:
	HBITMAP bmp;
	HDC bmpHdc;
	HWND parent;
	int x;
	int y;
	int forcedWidth;
	int forcedHeight;
	channel * rawData;
	unsigned int scanline;
	unsigned int width;
	unsigned int height;
	bool rawUpdated;
private:
	void convertToBitmap(const char * fileName);
public:
	Image(const char * file, HWND window = gui::GUI::useWindow());
	Image(const char * file, int x, int y, HWND window = gui::GUI::useWindow());
	Image(int width, int height, int x, int y, HWND window = gui::GUI::useWindow());
	Image();
	~Image();
	bool setPixel(int x, int y, Color c);
	Color getPixel(int x, int y);
	operator HBITMAP() { return bmp; }
	HBITMAP getHandle() { return bmp; }
	bool saveBmp(const char * fileName);
	void setPosition(int x, int y);
	void setConstrains(int forcedWidth, int forcedHeight) { this->forcedWidth = forcedWidth; this->forcedHeight = forcedHeight; }
	void toMonochrome();
	int getForcedWidth() { return forcedWidth; }
	int getForcedHeight() { return forcedHeight; }
	int getX() { return x; }
	int getY() { return y; }
	bool updatedSinceLoad() { return rawUpdated; }
	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }
	channel * getRawData() { return rawData; }
};
class Canvas {
private:
	std::vector<Image *> images;
	std::vector<Text *> labels;
	std::vector<Drawable *> objects;
	HWND parentWindow;
public:
	Canvas(HWND parent = gui::GUI::useWindow()) : parentWindow(parent) {}
	~Canvas();
	void addImage(Image * bmp);
	void addText(Text * txt);
	void removeText(Text * txt);
	void removeImage(Image * bmp);
	void addDrawable(Drawable * object);
	void handleMessages(UINT msg, WPARAM w, LPARAM l);
	void draw();
	void draw(HDC hdc);
	std::vector<Image *> * getImages() { return &images; }
	std::vector<Text *> * getLabels() { return &labels; }

};