#pragma once
#include <Canvas.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define radians(x) ((x) * (M_PI / 180.0))
#include "res.h"
#include <GUIS.h>
#include <sstream>
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
	int dash;
	int v;
};
POINT matrixMultiply(float * matrix, POINT vector);
float findSkewAngle(Image * img, POINT * origin);
void rotateImage(Image * img, float theta, POINT origin);
std::vector<Square> getCharacterLocations(Image * img);
char * identifyLetters(Image * img, std::vector<Square> locations);
Color bilinearInterpolation(Pixel q1, Pixel q2, Pixel q3, Pixel q4, POINT x);
CharacterFeatures getImageScore(Image * img);