#include "Image.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION
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

Image * Image::newImgFrom(Image * img, int x, int y, int width, int height)
{
	Image * newImg = new Image(width, height, x, y, gui::GUI::useWindow());
	for (int i = x; i < x + width; i++) {
		int newX = i - x;
		for (int j = y; j < y + height; j++) {
			int newY = j - y;
			newImg->setPixel(newX, newY, img->getPixel(x, y));
		}
	}
	return newImg;
}

Image::Image(const char * file, HWND window) : rawUpdated(false)
{
	int size = strlen(file);
	if (file[size - 4] != '.' || file[size - 3] != 'b' || file[size - 2] != 'm' || file[size - 1] != 'p') {
		convertToBitmap(file);
		bmp = (HBITMAP)LoadImage(NULL, "temp.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	else
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

Image::Image(gui::Resource res, HWND window)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	memcpy_s(&fileHeader, sizeof(BITMAPFILEHEADER), res.data, sizeof(BITMAPFILEHEADER));
	memcpy_s(&infoHeader, sizeof(BITMAPINFOHEADER), res.data + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
	width = infoHeader.biWidth;
	height = infoHeader.biHeight;
	forcedWidth = width;
	forcedHeight = height;
	scanline = infoHeader.biSizeImage / infoHeader.biHeight;
	BITMAPINFO bmi;
	bmi.bmiHeader = infoHeader;
	HDC dc = GetDC(gui::GUI::useWindow());
	bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&rawData, NULL, NULL);
	if (bmp == NULL) printf("Create DIB section failed %d \n", GetLastError());
	if (SetDIBits(dc, bmp, 0, infoHeader.biHeight, res.data + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER), &bmi, DIB_RGB_COLORS) == 0) printf("Set Dibits failed %d \n", GetLastError());
	if (GetDIBits(dc, bmp, 0, infoHeader.biHeight, &rawData, &bmi, DIB_RGB_COLORS) == 0) printf("Get DIbits failed %d \n", GetLastError());
}

Image::Image(int width, int height, int x, int y, HWND window) : width(width), height(height), x(x), y(y)
{
	scanline = width * 3;
	int padding = 0;
	while ((scanline + padding) % 4 != 0) padding++;
	scanline += padding;
	forcedHeight = height;
	forcedWidth = width;

	BITMAPINFOHEADER bi;
	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
	bi.biBitCount = 24;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;
	bi.biCompression = 0;
	bi.biHeight = height;
	bi.biPlanes = 1;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biSizeImage = scanline * height;
	bi.biXPelsPerMeter = 2800;
	bi.biYPelsPerMeter = 2800;
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader = bi;
	if (IsWindow(gui::GUI::useWindow()) == 0) printf("Window handle not valid! \n");
	HDC dc = GetDC(gui::GUI::useWindow());
	bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&rawData, NULL, NULL);
	if (bmp == NULL) printf("Failed to create DIB section in constructor %d \n", GetLastError());
	rawUpdated = true;
	ReleaseDC(gui::GUI::useWindow(), dc);
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

void Image::scaleTo(int width, int height)
{
	int originalWidth = this->width;
	int originalHeight = this->height;
	double factorX = (double)width / this->width;
	double factorY = (double)height / this->height;
	Color * originalData = new Color[this->width * this->height];
	for (int i = 0; i < this->width * this->height; i++) {
		int x = i % this->width;
		int y = i / this->width;
		originalData[i] = getPixel(x, y);
	}
	resize(width, height);
	if (factorX < 1 && factorY < 1) {
		int * avgR = new int[25];
		int * avgG = new int[25];
		int * avgB = new int[25];
		for (int i = 0; i < originalWidth; i++) {
			for (int j = 0; j < originalHeight; j++) {
				avgR[(int)round((factorY * j) * 5 + (factorX * i))] += originalData[j * originalWidth + i].r;
				avgG[(int)round((factorY * j) * 5 + (factorX * i))] += originalData[j * originalWidth + i].g;
				avgB[(int)round((factorY * j) * 5 + (factorX * i))] += originalData[j * originalWidth + i].b;
			}
		}
		for (int i = 0; i < 25; i++) {
			int x = i % 5;
			int y = i / 5;
			avgR[i] /= 25;
			avgG[i] /= 25;
			avgB[i] /= 25;
			setPixel(x, y, { (channel)avgR[i], (channel)avgG[i], (channel)avgB[i] });
		}
	}
	delete[] originalData;
}