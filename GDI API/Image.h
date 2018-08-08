#pragma once
#include "GUIS.h" 
#include <vector>
typedef unsigned char channel;
struct Color {
	channel r;
	channel g;
	channel b;
	int avg() { return ((int)r + g + b) / 3; }
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
	int * integralImage;
private:
	float lerp(float s, float e, float t) { return s + (e - s)*t; }
	float blerp(float c00, float c10, float c01, float c11, float tx, float ty) { return lerp(lerp(c00, c10, tx), lerp(c01, c11, tx), ty); }
public:
	Image(const char * file, HWND window = gui::GUI::useWindow());
	Image(const char * file, int x, int y, HWND window = gui::GUI::useWindow());
	Image(gui::Resource res, HWND window = gui::GUI::useWindow());
	Image(int width, int height, int x, int y, HWND window = gui::GUI::useWindow());
	Image(int width, int height) : Image(width, height, 0, 0, gui::GUI::useWindow()) {};
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
	void toGreyscale();
	int getForcedWidth() { return forcedWidth; }
	int getForcedHeight() { return forcedHeight; }
	int getX() { return x; }
	int getY() { return y; }
	bool updatedSinceLoad() { return rawUpdated; }
	unsigned int getWidth() { return width; }
	unsigned int getHeight() { return height; }
	channel * getRawData() { return rawData; }
	int integralImageValue(int x, int y);
	void resize(int width, int height); //simply resizes the buffer, nothing more
	void scaleTo(int width, int height);
	Image * scale(int width, int height);
	int getScanline() { return scanline; }
	void clearBmp(channel c) { memset(rawData, c, scanline * height); rawUpdated = true; }
};
class ImgPtr {
private:
	Image * img;
public:
	Image * operator->() { return img; }
	ImgPtr & operator=(Image * i) { img = i; return *this; }
	bool operator==(Image * i) { return i == img; }
	bool operator==(ImgPtr i) { return i.img == img; }
	~ImgPtr() { delete img; }
	ImgPtr(Image * img) : img(img) {};
	ImgPtr() : img(nullptr) {};
	Image * data() { return img; }
};
Color biLerp(POINT * points, Image * source, POINT p);