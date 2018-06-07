#pragma once
#include <Canvas.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define radians(x) ((x) * (M_PI / 180.0))
POINT matrixMultiply(float * matrix, POINT vector);
float findSkewAngle(Image * img, POINT * origin);
void rotateImage(Image * img, float theta, POINT origin);