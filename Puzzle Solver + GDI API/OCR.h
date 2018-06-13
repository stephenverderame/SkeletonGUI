#pragma once
#include <Canvas.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define radians(x) ((x) * (M_PI / 180.0))
#include "res.h"
#include <GUIS.h>
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
POINT matrixMultiply(float * matrix, POINT vector);
float findSkewAngle(Image * img, POINT * origin);
void rotateImage(Image * img, float theta, POINT origin);
std::vector<Square> getCharacterLocations(Image * img);
char * identifyLetters(Image * img, std::vector<Square> locations);
Color bilinearInterpolation(Pixel q1, Pixel q2, Pixel q3, Pixel q4, POINT x);