#pragma once
#include <cstdio>
#include <memory.h>
typedef unsigned char channel;
struct Color {
	channel red;
	channel green;
	channel blue;
	Color(channel red, channel green, channel blue) : red(red), green(green), blue(blue) {};
	Color() : red(0), green(0), blue(0) {};
};
class Bitmap {
private:
#pragma pack(2)
	struct {
		char h1 = 'B';
		char h2 = 'M';
		long fileSize;
		long r = 0;
		long dataOffset = 54;
		//--------- info header
		long size = 40;
		long width;
		long height;
		short planes = 1;
		short bitsPerPixel = 24;
		long compression = 0;
		long imageSize;
		long xPixelPerMeter;
		long yPixelPerMeter;
		long colorsUsed = 0;
		long importantColors = 0;
	}header_data;
#pragma pack(pop)
	FILE * file;
	unsigned char * data;
	char * filename;
public:
	Bitmap() {

	}
	Bitmap(char * name, unsigned int width, unsigned int height) : filename(name) {
		header_data.width = width;
		header_data.height = height;
		header_data.imageSize = width * height * 3;
		data = new unsigned char[header_data.imageSize];
		memset(data, 0, header_data.imageSize);
		header_data.fileSize = header_data.dataOffset + header_data.imageSize;
		header_data.xPixelPerMeter = 2, 835;
		header_data.yPixelPerMeter = 2, 835;
	}
	~Bitmap() {
		if (data != NULL)
			delete[] data;
		if (file != NULL)
			fclose(file);
	}
	void setPixel(unsigned int x, unsigned int y, channel r, channel g, channel b) {
		if (data == NULL) return;
		data[(y * header_data.width * 3) + (x * 3)] = b;
		data[(y * header_data.width * 3) + (x * 3) + 1] = g;
		data[(y * header_data.width * 3) + (x * 3) + 2] = r;
	}
	void saveBmp() {
		if (data == NULL) return;
		fopen_s(&file, filename, "wb");
		fwrite(&header_data, 54, 1, file);
		fwrite(data, 1, header_data.imageSize, file);
		fclose(file);
	}
	void readBmp(char * filename) {
		fopen_s(&file, filename, "rb");
		if (file == NULL) return;
		fread(&header_data, 54, 1, file);
		if (header_data.h1 != 'B' || header_data.h2 != 'M') return;
		if (data != NULL) delete[] data;
		data = new unsigned char[header_data.imageSize];
		fread(data, 1, header_data.imageSize, file);
		fclose(file);
	}
	void readBmp() {
		fopen_s(&file, filename, "rb");
		if (file == NULL) return;
		fread(&header_data, 54, 1, file);
		if (header_data.h1 != 'B' || header_data.h2 != 'M') return;
		if (data != NULL) delete[] data;
		data = new unsigned char[header_data.imageSize];
		fread(data, 1, header_data.imageSize, file);
		fclose(file);
	}
	void changeFile(char * name) { filename = name; }
	Color getPixel(unsigned int x, unsigned int y) {
		Color color;
		if (data == NULL) return color;
		color.blue = data[(y * header_data.width * 3) + (x * 3)];
		color.green = data[(y * header_data.width * 3) + (x * 3) + 1];
		color.red = data[(y * header_data.width * 3) + (x * 3) + 2];
		return color;
	}
};