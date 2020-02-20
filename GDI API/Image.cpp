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
Image::Image(const char * file, HWND window) : rawUpdated(false), integralImage(nullptr)
{
	int size = strlen(file);
	if (file[size - 4] != '.' || tolower(file[size - 3]) != 'b' || tolower(file[size - 2]) != 'm' || tolower(file[size - 1]) != 'p') {
		int width, height, components;
		unsigned char * data = stbi_load(file, &width, &height, &components, NULL);
		resize(width, height);
		for (int i = 0; i < width * height; i++) {
			int x = i % width;
			int y = i / width;
			Color c = { data[i * components], data[i * components + 1], data[i * components + 2] };
			setPixel(x, y, c);
		}
		stbi_image_free(data);

	}
	else {
		bmp = (HBITMAP)LoadImage(NULL, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		if (bmp == NULL) printf("BMP failed to load %s \n", file);
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

}

Image::Image(const char * file, int x, int y, HWND window) : Image(file, window)
{
	this->x = x;
	this->y = y;
}

Image::Image(gui::Resource res, HWND window) : integralImage(nullptr)
{
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	memcpy_s(&fileHeader, sizeof(BITMAPFILEHEADER), res.data.data(), sizeof(BITMAPFILEHEADER));
	memcpy_s(&infoHeader, sizeof(BITMAPINFOHEADER), res.data.data() + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
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
	if (SetDIBits(dc, bmp, 0, infoHeader.biHeight, res.data.data() + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER), &bmi, DIB_RGB_COLORS) == 0) printf("Set Dibits failed %d \n", GetLastError());
	if (GetDIBits(dc, bmp, 0, infoHeader.biHeight, &rawData, &bmi, DIB_RGB_COLORS) == 0) printf("Get DIbits failed %d \n", GetLastError());
	ReleaseDC(gui::GUI::useWindow(), dc);
}

Image::Image(int width, int height, int x, int y, HWND window) : width(width), height(height), x(x), y(y), integralImage(nullptr)
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

Image::Image(const Image & other) : Image(other.width, other.height)
{
	for (int i = 0; i < other.width * other.height; i++) {
		int x = i % other.width;
		int y = i / other.width;
		setPixel(x, y, other.getPixel(x, y));
	}
}

Image::Image(HBITMAP bmp, HWND window)
{
	this->bmp = bmp;
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

Image & Image::operator=(const Image & other)
{
	resize(other.width, other.height);
	for (int i = 0; i < other.width * other.height; i++) {
		int x = i % other.width;
		int y = i / other.width;
		setPixel(x, y, other.getPixel(x, y));
	}
	if (integralImage != nullptr) {
		delete[] integralImage;
		integralImage = nullptr;
		if (other.integralImage != nullptr) {
			integralImage = new int[other.width * other.height];
			memcpy(integralImage, other.integralImage, sizeof(int) * other.width * other.height);
		}
	}
	return *this;
}

Image::Image() : bmp(NULL), integralImage(nullptr)
{
}

Image::~Image()
{
	DeleteDC(bmpHdc);
	DeleteObject(bmp);
	if (integralImage != nullptr)
		delete[] integralImage;
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

Color Image::getPixel(int x, int y) const
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

void Image::toGreyscale()
{
	for (int i = 0; i < width * height; i++) {
		int x = i % width;
		int y = i / width;
		Color c = getPixel(x, y);
		if (c.r + c.b + (int)c.g > 384)
			setPixel(x, y, { 255, 255, 255 });
		else
			setPixel(x, y, { 0, 0, 0 });
	}
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
	if(bmp != NULL)
		DeleteObject(bmp);
	HDC dc = GetDC(gui::GUI::useWindow());
	bmp = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (void**)&rawData, NULL, NULL);
	if (bmp == NULL) printf("CreateDIBSection failed! %d \n", GetLastError());
	ReleaseDC(gui::GUI::useWindow(), dc);
	if (rawData == nullptr) printf("rawData is null \n");
	rawUpdated = true;
}
#define getOPixel(point) originalData->getPixel(point.x, point.y)
void Image::scaleTo(int width, int height)
{
	int originalWidth = this->width;
	int originalHeight = this->height;
//	double factorX = (double)width / this->width;
//	double factorY = (double)height / this->height;
	Image * originalData = new Image(this->width, this->height, 0, 0);
	for (int i = 0; i < this->width * this->height; i++) {
		int x = i % this->width;
		int y = i / this->width;
		originalData->setPixel(x, y, getPixel(x, y));
	}
	resize(width, height);
//	if (/*factorX < 1 && factorY < 1*/ true) {
		double factorX = (double)originalWidth / width;
		double factorY = (double)originalHeight / height;
		float * xCenters = new float[originalWidth];
		float * yCenters = new float[originalHeight];
		float * newXCenters = new float[width];
		float * newYCenters = new float[height];
		for (int i = 0; i < originalWidth; i++)
			xCenters[i] = i + 0.5;
		for (int i = 0; i < width; i++)
			newXCenters[i] = (factorX * i) + (factorX / 2.0);
		for (int i = 0; i < height; i++)
			newYCenters[i] = (factorY * i) + (factorY / 2.0);
		for (int i = 0; i < originalHeight; i++)
			yCenters[i] = i + 0.5;

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				POINT p[4];
				float pDists[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
				float xDists[4];
				float yDists[4];
				for (int i = 0; i < originalWidth; i++) {
					for (int j = 0; j < originalHeight; j++) {
						float xDist = abs(xCenters[i] - newXCenters[x]);
						float yDist = abs(yCenters[j] - newYCenters[y]);
						float dist = sqrt(xDist * xDist + yDist * yDist);
						if (xCenters[i] < newXCenters[x] && yCenters[j] < newYCenters[y] && dist < pDists[0]) {
							p[0] = { i, j };
							pDists[0] = dist;
							xDists[0] = xDist;
							yDists[0] = yDist;
						}
						else if (xCenters[i] > newXCenters[x] && yCenters[j] < newYCenters[y] && dist < pDists[1]) {
							p[1] = { i, j };
							pDists[1] = dist;
							xDists[1] = xDist;
							yDists[1] = yDist;
						}
						else if (xCenters[i] < newXCenters[x] && yCenters[j] > newYCenters[y] && dist < pDists[2]) {
							p[2] = { i, j };
							pDists[2] = dist;
							xDists[2] = xDist;
							yDists[2] = yDist;
						}
						else if (xCenters[i] > newXCenters[x] && yCenters[j] > newYCenters[y] && dist < pDists[3]) {
							p[3] = { i, j };
							pDists[3] = dist;
							xDists[3] = xDist;
							yDists[3] = yDist;
						}
					}
				}
//				setPixel(x, y, biLerp(p, originalData, { x, y }));
				float r1 = (xDists[3] / (xDists[2] + xDists[3])) * getOPixel(p[2]).r + (xDists[2] / (xDists[2] + xDists[3])) * getOPixel(p[3]).r; //yDist[3]
				float r2 = (xDists[1] / (xDists[0] + xDists[1])) * getOPixel(p[0]).r + (xDists[0] / (xDists[0] + xDists[1])) * getOPixel(p[1]).r; //yDist[0]
				float interpolated = (yDists[0] / (yDists[0] + yDists[3])) * r1 + (yDists[3] / (yDists[0] + yDists[3])) * r2;
				channel r = (channel)round(interpolated);

				r1 = (xDists[3] / (xDists[2] + xDists[3])) * getOPixel(p[2]).g + (xDists[2] / (xDists[2] + xDists[3])) * getOPixel(p[3]).g; //yDist[3]
				r2 = (xDists[1] / (xDists[0] + xDists[1])) * getOPixel(p[0]).g + (xDists[0] / (xDists[0] + xDists[1])) * getOPixel(p[1]).g; //yDist[0]
				interpolated = (yDists[0] / (yDists[0] + yDists[3])) * r1 + (yDists[3] / (yDists[0] + yDists[3])) * r2;
				channel g = (channel)round(interpolated);

				r1 = (xDists[3] / (xDists[2] + xDists[3])) * getOPixel(p[2]).b + (xDists[2] / (xDists[2] + xDists[3])) * getOPixel(p[3]).b; //yDist[3]
				r2 = (xDists[1] / (xDists[0] + xDists[1])) * getOPixel(p[0]).b + (xDists[0] / (xDists[0] + xDists[1])) * getOPixel(p[1]).b; //yDist[0]
				interpolated = (yDists[0] / (yDists[0] + yDists[3])) * r1 + (yDists[3] / (yDists[0] + yDists[3])) * r2;
				channel b = (channel)round(interpolated);

				setPixel(x, y, { r, g, b });
			}

		}
/*		for (int x = 0; x < width; x++) {
			float close1 = FLT_MAX, close2 = FLT_MAX;
			int close1ID, close2ID;
			for (int i = 0; i < originalWidth; i++) {
				float dist = abs(newXCenters[x] - xCenters[i]);
				if (dist < close1) {
					close1 = dist;
					close1ID = i;
				}
				else if (dist < close2) {
					close2 = dist;
					close2ID = i;
				}
			}
			for (int y = 0; y < height; y++) {
				float closey1 = FLT_MAX, closey2 = FLT_MAX;
				int closey1ID, closey2ID;
				for (int i = 0; i < originalHeight; i++) {
					float dist = abs(newYCenters[y] - yCenters[i]);
					if (dist < closey1) {
						closey1 = dist;
						closey1ID = i;
					}
					else if (dist < closey2) {
						closey2 = dist;
						closey2ID = i;
					}
				}
				float interpolatedAvg = originalData->getPixel(close1ID, closey1ID).avg() * close2 * closey2 + originalData->getPixel(close2ID, closey1ID).avg() * close1 * closey2 + 
					originalData->getPixel(close1ID, closey2ID).avg() * close2 * closey1 + originalData->getPixel(close2ID, closey2ID).avg() * close1 * closey1;
				channel avg = (channel)round(interpolatedAvg);
				setPixel(x, y, { avg, avg, avg });
			}

		}*/
		delete[] xCenters;
		delete[] yCenters;
		delete[] newXCenters;
		delete[] newYCenters;
//	}
	delete originalData;
/*	int originalWidth = this->width;
	int originalHeight = this->height;
	double factorX = (double)width / this->width;
	double factorY = (double)height / this->height;
	Image * originalData = new Image(this->width, this->height, 0, 0);
	for (int i = 0; i < this->width * this->height; i++) {
		int x = i % this->width;
		int y = i / this->width;
		originalData->setPixel(x, y, getPixel(x, y));
	}
	resize(width, height);
	if (factorX < 1 && factorY < 1) {
		std::vector<std::vector<POINT>> pointMapping;
		pointMapping.resize(width * height);
		for (int i = 0; i < originalWidth; i++) {
			for (int j = 0; j < originalHeight; j++) {
				int newX = round(factorX * i);
				int newY = round(factorY * j);
				newY = newY == height ? height - 1 : newY;
				newX = newX == width ? width - 1 : newX;
				pointMapping[newY * width + newX].push_back({ i, j });
			}
		}
/*		for (int i = 0; i < width * height; i++) {
			int x = i % width;
			int y = i / width;
			POINT f[4] = { { INT_MAX, INT_MAX }, { INT_MIN, INT_MAX }, { INT_MAX, INT_MIN }, { INT_MIN, INT_MIN } };
			for (int j = 0; j < pointMapping[i].size(); j++) {
				POINT newPoint = pointMapping[i][j];
				if (newPoint.x <= x && newPoint.y <= y && newPoint.x < f[0].x && newPoint.y < f[0].y)
					f[0] = newPoint;
				else if (newPoint.x >= x && newPoint.y <= y && newPoint.x > f[1].x && newPoint.y < f[1].y)
					f[1] = newPoint;
				else if (newPoint.x <= x && newPoint.y >= y && newPoint.x < f[2].x && newPoint.y > f[2].y)
					f[2] = newPoint;
				else if (newPoint.x >= x && newPoint.y >= y && newPoint.x > f[3].x && newPoint.y > f[3].y)
					f[3] = newPoint;
			}
			printf("(%d, %d)     (%d, %d)", f[0].x, f[0].y, f[1].x, f[1].y);
			printf("(%d, %d)     (%d, %d) \n", f[2].x, f[2].y, f[3].x, f[3].y);
			setPixel(x, y, biLerp(f, originalData, { x, y }));
		}
		for (int i = 0; i < width * height; i++) {
			int x = i % width;
			int y = i / width;
			int sumr = 0, sumg = 0, sumb = 0;
			for (int j = 0; j < pointMapping[i].size(); j++) {
				sumr += originalData->getPixel(pointMapping[i][j].x, pointMapping[i][j].y).r;
				sumg += originalData->getPixel(pointMapping[i][j].x, pointMapping[i][j].y).g;
				sumb += originalData->getPixel(pointMapping[i][j].x, pointMapping[i][j].y).b;
			}
			sumr /= pointMapping[i].size();
			sumg /= pointMapping[i].size();
			sumb /= pointMapping[i].size();
			setPixel(x, y, { (channel)sumr, (channel)sumg, (channel)sumb });
		}
	}
	delete originalData;*/
}

Image * Image::scale(int width, int height)
{
	Image * img = new Image(width, height);
	for (int i = 0; i < width * height; i++) {
		int x = i % width;
		int y = i / width;
		float gx = x / (float)width * (this->width - 1);
		float gy = y / (float)height * (this->height - 1);
		int gxi = gx;
		int gyi = gy;
		Color result;
		Color c00 = getPixel(gxi, gyi);
		Color c10 = getPixel(gxi + 1, gyi);
		Color c01 = getPixel(gxi, gyi + 1);
		Color c11 = getPixel(gxi + 1, gyi + 1);
		result.r = (channel)blerp(c00.r, c10.r, c01.r, c11.r, gx - gxi, gy - gyi);
		result.g = (channel)blerp(c00.g, c10.g, c01.g, c11.g, gx - gxi, gy - gyi);
		result.b = (channel)blerp(c00.b, c10.b, c01.b, c11.b, gx - gxi, gy - gyi);
		img->setPixel(x, y, result);
	}
	return img;
}
Color biLerp(POINT * points, Image * source, POINT p)
{
	Color pColor;
	int x1 = points[0].x, x2 = points[1].x, y1 = points[3].y, y2 = points[1].y;
	double r1 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[2].x, points[2].y).r + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[3].x, points[3].y).r;
	double r2 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[0].x, points[0].y).r + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[1].x, points[1].y).r;
	pColor.r = (channel)(((y2 - p.y) / (y2 - y1)) * r1 + ((p.y - y1) / (y2 - y1)) * r2);
	double g1 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[2].x, points[2].y).g + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[3].x, points[3].y).g;
	double g2 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[0].x, points[0].y).g + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[1].x, points[1].y).g;
	pColor.g = (channel)(((y2 - p.y) / (y2 - y1)) * g1 + ((p.y - y1) / (y2 - y1)) * g2);
	double b1 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[2].x, points[2].y).b + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[3].x, points[3].y).b;
	double b2 = ((x2 - p.x) / (x2 - x1)) * source->getPixel(points[0].x, points[0].y).b + ((p.x - x1) / (x2 - x1)) * source->getPixel(points[1].x, points[1].y).b;
	pColor.b = (channel)(((y2 - p.y) / (y2 - y1)) * b1 + ((p.y - y1) / (y2 - y1)) * b2);
	return pColor;
}
