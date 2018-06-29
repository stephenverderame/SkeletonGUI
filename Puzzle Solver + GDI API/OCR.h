#pragma once
#include <Canvas.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define radians(x) ((x) * (M_PI / 180.0))
#include "res.h"
#include <GUIS.h>
#include <sstream>
#include <fstream>
#include <time.h>
struct Square {
	int x;
	int y;
	int width;
	int height;
};
struct Rect {
	POINT topLeft;
	POINT bottomRight;
	operator Square();
};
struct Space {
	int start;
	int size;
};
struct Pixel {
	int x;
	int y;
	Color c;
};
struct Letter {
	int row;
	int column;
	char letter;
};
struct Bounds {
	float min;
	float max;
};
struct KnownSample {
	Image * image;
	char letter;
	~KnownSample() { delete image; }
	operator Image*() { return image; }
	Image * operator->() { return image; }
};
class SearchGrid {
private:
	std::vector <Letter *> letters;
	int row0Y;
	int column0X;
	std::pair<int, int> lastRow;
	std::pair<int, int> lastColumn;
	int maxRows = 0;
	int maxColumns = 0;
public:
	void addLetter(char c, int x, int y);
	~SearchGrid();
	void iterateRowbyRow();
};
struct CharacterFeatures {
	int elbow1;
	int elbow2;
	int dash1;
	int dash2;
	int v;
	int threshold;
};
POINT matrixMultiply(float * matrix, POINT vector);
float findSkewAngle(Image * img, POINT * origin, Bounds * skewBounds = NULL);
void rotateImage(Image * img, float theta, POINT origin);
std::vector<Square> getCharacterLocations(Image * img);
SearchGrid identifyLetters(Image * img, std::vector<Square> locations);
void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, Image * img, int firstKnown = 0);
Color bilinearInterpolation(Pixel q1, Pixel q2, Pixel q3, Pixel q4, POINT x);
CharacterFeatures getImageScore(Image * img);