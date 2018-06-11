#include "Canvas.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif
struct arrayData {
	int width;
	int height;
	int * array;
};
int getPoint(int x, int y, arrayData data) {
	if (x < 0 || x >= data.width || y < 0 || y >= data.height) return 0;
	return data.array[y * data.width + x];
}

void Image::convertToBitmap(const char * fileName)
{
	int width, height, channels, widthBytes = 0;
	int padding = 0;
	unsigned char * data = stbi_load(fileName, &width, &height, &channels, 0);
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	ZeroMemory(&fileHeader, sizeof(fileHeader));
	ZeroMemory(&infoHeader, sizeof(infoHeader));
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (width * height * 3);
	fileHeader.bfOffBits = 54;
	fileHeader.bfType = 0x4d42;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = width;
	infoHeader.biHeight = height;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = 24;
	infoHeader.biCompression = 0;
	infoHeader.biClrImportant = 0;
	infoHeader.biSizeImage = width * height * 3;
	infoHeader.biXPelsPerMeter = 0x0ec4;
	infoHeader.biYPelsPerMeter = 0x0ec4;
	infoHeader.biClrUsed = 0;
	FILE * file;
	fopen_s(&file, "temp.bmp", "wb");
	fwrite(&fileHeader, sizeof(fileHeader), 1, file);
	fwrite(&infoHeader, sizeof(infoHeader), 1, file);
	fwrite(data, 1, width * height * 3, file);
	fclose(file);
	stbi_image_free(data);
}

Image::Image(const char * file, HWND window) : rawUpdated(false)
{
	int size = strlen(file);
	if (file[size - 4] != '.' || file[size - 3] != 'b' || file[size - 2] != 'm' || file[size - 1] != 'p') {
		convertToBitmap(file);
		bmp = (HBITMAP)LoadImage(NULL, "temp.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}else
		bmp = (HBITMAP)LoadImage(NULL, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	this->parent = window;
	BITMAP bm;
	ZeroMemory(&bm, sizeof(bm));
	GetObject(bmp, sizeof(bm), &bm);
	rawData = (channel*)bm.bmBits;
	scanline = bm.bmWidthBytes;
	width = bm.bmWidth;
	height = bm.bmHeight;
	forcedWidth = width;
	forcedHeight = height;
	
}

Image::Image(const char * file, int x, int y, HWND window) : rawUpdated(false)
{
	int size = strlen(file);
	if (file[size - 4] != '.' || file[size - 3] != 'b' || file[size - 2] != 'm' || file[size - 1] != 'p') {
		convertToBitmap(file);
		bmp = (HBITMAP)LoadImage(NULL, "temp.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	else
		bmp = (HBITMAP)LoadImage(NULL, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	this->parent = window;
	this->x = x;
	this->y = y;
	BITMAP bm;
	ZeroMemory(&bm, sizeof(bm));
	GetObject(bmp, sizeof(bm), &bm);
	rawData = (channel*)bm.bmBits;
	scanline = bm.bmWidthBytes;
	width = bm.bmWidth;
	height = bm.bmHeight;
	forcedWidth = width;
	forcedHeight = height;
}

Image::Image(int width, int height, int x, int y, HWND window) : width(width), height(height), x(x), y(y), scanline(width * 3), bmp(NULL)
{
	rawData = new channel[scanline * height];
	memset(rawData, 0, scanline * height);
}

Image::Image() : bmp(NULL)
{
}

Image::~Image()
{
	DeleteDC(bmpHdc);
	if (rawData != nullptr)
		delete[] rawData;
}

bool Image::setPixel(int x, int y, Color c)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return false;
	rawData[(height - y - 1) * scanline + x * 3] = c.b;
	rawData[(height - y - 1) * scanline + x * 3 + 1] = c.g;
	rawData[(height - y - 1) * scanline + x * 3 + 2] = c.r;
	rawUpdated = true;
	return true;
}

Color Image::getPixel(int x, int y)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return{ 0, 0, 0 };
	Color color;
	color.b = rawData[(height - y - 1) * scanline + x * 3];
	color.g = rawData[(height - y - 1) * scanline + x * 3 + 1];
	color.r = rawData[(height - y - 1) * scanline + x * 3 + 2];
	return color;
}

bool Image::saveBmp(const char * fileName)
{
	BITMAP bm;
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	ZeroMemory(&fileHeader, sizeof(fileHeader));
	ZeroMemory(&infoHeader, sizeof(infoHeader));
	ZeroMemory(&bm, sizeof(bm));
	GetObject(bmp, sizeof(bm), &bm);
	fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bm.bmWidthBytes * bm.bmHeight);
	fileHeader.bfOffBits = 54;
	fileHeader.bfType = 0x4d42;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;

	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth = bm.bmWidth;
	infoHeader.biHeight = bm.bmHeight;
	infoHeader.biPlanes = 1;
	infoHeader.biBitCount = bm.bmBitsPixel;
	infoHeader.biCompression = 0;
	infoHeader.biClrImportant = 0;
	infoHeader.biSizeImage = bm.bmWidthBytes * bm.bmHeight;
	infoHeader.biXPelsPerMeter = 0x0ec4;
	infoHeader.biYPelsPerMeter = 0x0ec4;
	infoHeader.biClrUsed = 0;
	unsigned long written;
	FILE * file;
	fopen_s(&file, fileName, "wb");
	fwrite(&fileHeader, sizeof(fileHeader), 1, file);
	fwrite(&infoHeader, sizeof(infoHeader), 1, file);
	fwrite(rawData, 1, bm.bmWidthBytes * bm.bmHeight, file);
	fclose(file);
	return true;
}

void Image::setPosition(int x, int y)
{
	RECT rect;  
	GetClientRect(parent, &rect); 
	InvalidateRect(parent, &rect, TRUE); 
	this->x = x; 
	this->y = y;
}

void Image::toMonochrome()
{
	if (integralImage == nullptr) {
		integralImage = new int[width * height];
		arrayData data{ width, height, integralImage };
		for (int i = 0; i < width * height; i++) {
			int x = i % width;
			int y = i / width;
			integralImage[i] = getPixel(x, y).avg() + getPoint(x - 1, y, data) + getPoint(x, y - 1, data) - getPoint(x - 1, y - 1, data);
		}
	}
	arrayData data{ width, height, integralImage };
	for (int i = 0; i < width * height; i++) {
		int x = i % width;
		int y = i / width;
		int xp = min(width - 1, x + 11);
		int yp = min(height - 1, y + 11);
		int xm = max(0, x - 11);
		int ym = max(0, y - 11);
		int sum = getPoint(xp, yp, data) - getPoint(xm, yp, data) - getPoint(xp, ym, data) + getPoint(xm, ym, data);
		sum /= ((xp - xm) * (yp - ym));
		Color c = getPixel(x, y);
		int avg = (c.r + c.g + c.b) / 3;
		Color newColor = avg < sum ? Color{ 0, 0, 0 } : Color{ 255, 255, 255 };
		setPixel(x, y, newColor);
	}
/*	for (int i = 0; i < width * height; i++) {
		int x1 = i % width;
		int y1 = i / width;
		int sumr = 0, sumg = 0, sumb = 0;
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				if (x1 + x < 0 || x1 + x >= width || y1 + y < 0 || y1 + y > height) continue;
				sumr += getPixel(x1 + x, y1 + y).r;
				sumg += getPixel(x1 + x, y1 + y).g;
				sumb += getPixel(x1 + x, y1 + y).b;
			}
		}
		sumr /= 9; sumg /= 9; sumb /= 9;
		setPixel(x1, y1, { (channel)sumr, (channel)sumg, (channel)sumb });
	}*/
	delete[] integralImage;
	integralImage = nullptr;

}

int Image::integralImageValue(int x, int y)
{
	if (integralImage == nullptr) {
		integralImage = new int[width * height];
		arrayData data{ width, height, integralImage };
		for (int i = 0; i < width * height; i++) {
			int x = i % width;
			int y = i / width;
			integralImage[i] = getPixel(x, y).avg() + getPoint(x - 1, y, data) + getPoint(x, y - 1, data) - getPoint(x - 1, y - 1, data);
		}
	}
	if (x < 0 || x >= width || y < 0 || y >= height) return 0;
	return integralImage[y * width + x];
}

void Image::resize(int width, int height)
{
	this->width = width;
	this->scanline = width * 3;
	int padding = 0;
	while ((scanline + padding) % 4 != 0) padding++;
	scanline += padding;
	this->height = height;
	this->forcedWidth = width;
	this->forcedHeight = height;
	rawData = nullptr;
	if (integralImage != nullptr) {
		delete[] integralImage;
		integralImage = nullptr;
	}
	PAINTSTRUCT p;
	BITMAPINFOHEADER bi;
	bi.biBitCount = 24;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;
	bi.biCompression = 0;
	bi.biHeight = height;
	bi.biWidth = width;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biSizeImage = scanline * height;
	bi.biXPelsPerMeter = 2800;
	bi.biYPelsPerMeter = 2800;
	bi.biPlanes = 1;
	BITMAPINFO bmi;
	bmi.bmiHeader = bi;
	DeleteObject(bmp);
	HDC dc = GetDC(gui::GUI::useWindow());
	bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&rawData, NULL, NULL);
	if (bmp == NULL) printf("CreateDIBSection failed! %d \n", GetLastError());
	ReleaseDC(gui::GUI::useWindow(), dc);
	if (rawData == nullptr) printf("rawData is null \n");
	rawUpdated = true;
}

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

