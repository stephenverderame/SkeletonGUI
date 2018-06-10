#include "OCR.h"
POINT matrixMultiply(float * matrix, POINT vector) {
	POINT result = { 0, 0 };
	result.x = matrix[0] * vector.x + matrix[1] * vector.y;
	result.y = matrix[2] * vector.x + matrix[3] * vector.y;
	return result;
}
float findSkewAngle(Image * img, POINT * origin) {
	int startY = 0;
	int startX = 0;
	int totalAvg = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) - img->integralImageValue(0, img->getHeight() - 1) - img->integralImageValue(img->getWidth() - 1, 0) + img->integralImageValue(0, 0);
	totalAvg /= (img->getWidth() * img->getHeight());
	printf("%d \n", totalAvg);
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		Color c = img->getPixel(x, y);
		if (c.avg() < 100) {
			int xp = max(x + (.15 * img->getWidth()), img->getWidth() - 1);
			int yp = max(y + (.15 * img->getHeight()), img->getHeight() - 1);
			int xm = min(x - (.15 * img->getWidth()), 0);
			int ym = min(y - (.15 * img->getHeight()), 0);
			int avg = img->integralImageValue(xp, yp) - img->integralImageValue(xm, yp) - img->integralImageValue(xp, ym) + img->integralImageValue(xm, ym);
			avg /= (xp - xm) * (yp - ym);
			avg = abs(avg);
			if (avg < totalAvg) {
				printf("Area avg: %d \n", avg);
				printf("Start (%d,%d) \n", x, y);
				startY = y;// +7;
				startX = x;
				break;
			}
		}
	}
	if (origin != NULL) *origin = { startX, startY };
	std::vector<POINT> line;
	int * accumulator = new int[360];
	memset(accumulator, 0, 360 * sizeof(int));
	int diagnol = sqrt(img->getWidth() * img->getWidth() + img->getHeight() * img->getHeight());
	for (int i = 0; i < diagnol; i++) {
		line.push_back({ startX + i, startY });
	}
	for (int theta = 0; theta < 360; theta++) {
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		for (int i = 0; i < img->getWidth(); i++) {
			POINT linePt = { line[i].x - startX, line[i].y - startY };
			POINT testPoint = matrixMultiply(rotationMatrix, linePt);
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img->getPixel(testPoint.x, testPoint.y).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img->getWidth() || testPoint.y < 0 || testPoint.y >= img->getHeight()) pixel = 0;
			accumulator[theta] += pixel;
		}
	}
	int maxPixels = INT_MIN;
	int indexMax = 0;
	for (int i = 0; i < 360; i++) {
		int old = maxPixels;
		maxPixels = max(maxPixels, accumulator[i]);
		if (old != maxPixels) indexMax = i;
	}
#pragma region extraPrecision
	int indexMaxMinor = 0;
	memset(accumulator, 0, 10 * sizeof(int));
	for (int minor = -5; minor < 5; minor++) {
		float theta = indexMax + (minor / 10.0);
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		for (int i = 0; i < img->getWidth(); i++) {
			POINT linePt = { line[i].x - startX, line[i].y - startY };
			POINT testPoint = matrixMultiply(rotationMatrix, linePt);
			testPoint.x += startX;
			testPoint.y += startY;
			int pixel = img->getPixel(testPoint.x, testPoint.y).avg() < 100 ? 1 : 0;
			if (testPoint.x < 0 || testPoint.x >= img->getWidth() || testPoint.y < 0 || testPoint.y >= img->getHeight()) pixel = 0;
			accumulator[minor + 5] += pixel;
		}
	}
	maxPixels = INT_MIN;
	for (int i = 0; i < 10; i++) {
		int old = maxPixels;
		maxPixels = max(maxPixels, accumulator[i]);
		if (old != maxPixels) indexMaxMinor = i;
	}
#pragma endregion
	float angle = indexMax + ((indexMaxMinor - 5) / 10.0);
	//Testing
	for (int i = 0; i < diagnol; i++) {
		float theta = angle;
		float rotationMatrix[] = {
			cos(radians(theta)), -sin(radians(theta)),
			sin(radians(theta)),  cos(radians(theta))
		};
		POINT linePt = { line[i].x - startX, line[i].y - startY };
		POINT drawPoint = matrixMultiply(rotationMatrix, linePt);
		drawPoint.x += startX;
		drawPoint.y += startY;
		if (drawPoint.x > 0 && drawPoint.x < img->getWidth() && drawPoint.y > 0 && drawPoint.y < img->getHeight()) {
			img->setPixel(drawPoint.x, drawPoint.y, Color{ 255, 0, 0 });
		}
	}
	img->setPixel(startX, startY, Color{ 0, 255, 0 });
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	printf("Original angle: %f \n", angle);
	//end testing
	delete[] accumulator;
	if (abs(angle - 90) < 20) {
		angle -= 90;
	}
	else if (angle > 90) {
		if (abs(180 - angle) < 90) {
			angle = 180 - angle;
			angle = -angle;
		}
	}
	return angle;

}
Square detectSearchBorder(Image * img)
{
	int * accumulator = new int[img->getWidth()];
	std::vector<Space> spaces;
	for (int i = 0; i < img->getWidth(); i++) {
		for (int y = 0; y < img->getHeight(); y++) {
			accumulator[i] = img->getPixel(i, y).avg();
		}
	}
	int lastDark = 0;
	for (int i = 0; i < img->getWidth(); i++) {
		if (accumulator[i] / img->getHeight() < 128) {
			if (i - lastDark > 1) spaces.push_back({ lastDark + 1, i - lastDark });
			lastDark = i;
		}
	}
	int avgSpaceSize = 0;
	for (Space s : spaces) {
		avgSpaceSize += s.size;
	}
	avgSpaceSize /= spaces.size();
}
#ifdef OLD_ROTATE
void rotateImage(Image * img, float theta, POINT origin)
{
	Color * buffer = new Color[img->getWidth() * img->getHeight()];
	//Debugging
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		buffer[i] = Color{ 0, 255, 0 };
	}
	//end debugging
	float rotationMatrix[] = {
		cos(radians(theta)),  -sin(radians(theta)),
		sin(radians(theta)),  cos(radians(theta))
	};
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		Color c = img->getPixel(x, y);
		POINT rtPt = { x - origin.x, y - origin.y };
		POINT rotated = matrixMultiply(rotationMatrix, rtPt);
		rotated.x += origin.x;
		rotated.y += origin.y;
		if (rotated.x > 0 && rotated.x < img->getWidth() && rotated.y > 0 && rotated.y < img->getHeight())
			buffer[rotated.y * img->getWidth() + rotated.x] = c;
	}
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		img->setPixel(x, y, buffer[i]);
	}
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] buffer;
}
#else
void rotateImage(Image * img, float theta, POINT origin)
{
	//resizing is WIP
	int diagnol = ceil(sqrt(img->getWidth() * img->getWidth() + img->getHeight() * img->getHeight()));
	Color * buffer = new Color[diagnol * diagnol];
	//Debugging
	for (int i = 0; i < diagnol * diagnol; i++) {
		buffer[i] = Color{ 0, 255, 0 };
	}
	//end debugging
	float rotationMatrix[] = {
		cos(radians(theta)),  -sin(radians(theta)),
		sin(radians(theta)),  cos(radians(theta))
	};
	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		Color c = img->getPixel(x, y);
		POINT rtPt = { x - origin.x, y - origin.y };
		POINT rotated = matrixMultiply(rotationMatrix, rtPt);
		rotated.x += origin.x;
		rotated.y += origin.y;
		rotated.x += (diagnol - img->getWidth()) / 2;
		rotated.y += (diagnol - img->getHeight()) / 2;
		if (rotated.x > 0 && rotated.x < diagnol && rotated.y > 0 && rotated.y < diagnol)
			buffer[rotated.y * diagnol + rotated.x] = c;
	}
	img->resize(diagnol, diagnol);
	PAINTSTRUCT p;
	HDC dc = BeginPaint(gui::GUI::useWindow(), &p);
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		channel * bits = img->getRawData();
		if (bits == nullptr) printf("Bits is null \n");
//		printf("Max num = %d \n", diagnol * diagnol * 4);
//		printf("Current num = %d \n", y * diagnol * 4 + x * 3);
//		bits[(diagnol - y - 1) * diagnol * 3 + x * 3] = buffer[i].b;
//		bits[(diagnol - y - 1) * diagnol * 3 + x * 3 + 1] = buffer[i].g;
//		bits[(diagnol - y - 1) * diagnol * 3 + x * 3 + 2] = buffer[i].r;
		img->setPixel(x, y, buffer[i]);
		//Color test = img->getPixel(x, y);
		//SetPixel(dc, x, y, RGB(test.r, test.g, test.b));
	}
	EndPaint(gui::GUI::useWindow(), &p);
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] buffer;
}
#endif
std::vector<Square> getCharacterLocations(Image * img, Square border)
{
	std::vector<Square> list;
}
Rect::operator Square()
{
	return{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}
