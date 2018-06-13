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
			int xp = min(x + (.15 * img->getWidth()), img->getWidth() - 1);
			int yp = min(y + (.15 * img->getHeight()), img->getHeight() - 1);
			int xm = max(x - (.15 * img->getWidth()), 0);
			int ym = max(y - (.15 * img->getHeight()), 0);
			int avg = img->integralImageValue(xp, yp) - img->integralImageValue(xm, yp) - img->integralImageValue(xp, ym) - img->integralImageValue(xm, ym);
			printf("Avg: %d \n", avg);
			avg /= (xp - xm) * (yp - ym);
			avg = abs(avg);
			if (avg < totalAvg) {
				printf("Area avg: %d \n", avg);
				printf("Start (%d,%d) \n", x, y);
				startY = y + 7; //puts point in the middle of most letters
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
//	img->setPixel(startX, startY, Color{ 0, 255, 0 });
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
std::vector<Square> getCharacterLocations(Image * img)
{
#pragma region columnSpaces
	int * accumulator = new int[img->getWidth()];
	memset(accumulator, 0, img->getWidth() * sizeof(int));
	std::vector<Space> spaces;
	std::vector<int> spaceSizes;
	int totalAvg = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) - img->integralImageValue(0, img->getHeight() - 1) - img->integralImageValue(img->getWidth() - 1, 0) + img->integralImageValue(0, 0);
	totalAvg /= (img->getWidth() * img->getHeight());
	printf("Total Avg: %d \n", totalAvg);
	for (int i = 0; i < img->getWidth(); i++) {
		for (int y = 0; y < img->getHeight(); y++) {
			accumulator[i] += img->getPixel(i, y).avg();
		}
	}
	int lastDark = 0;
	for (int i = 0; i <= img->getWidth(); i++) {
		if (i == img->getWidth()) {
			spaces.push_back({ lastDark + 1, i - lastDark });
			spaceSizes.push_back(i - lastDark);
			break;
		}
		if (accumulator[i] / (float)img->getHeight() < 230) {
			if (i - lastDark > 1) {
				spaces.push_back({ lastDark + 1, i - lastDark });
				spaceSizes.push_back(i - lastDark);
			}
			lastDark = i;
		}
	}
	int avgSpaceSize = 0;
	for (int i = 0; i < spaceSizes.size(); i++) {
		for (int j = 0; j < spaceSizes.size(); j++) {
			if (spaceSizes[j] < spaceSizes[i]) {
				int size = spaceSizes[i];
				spaceSizes[i] = spaceSizes[j];
				spaceSizes[j] = size;
			}
		}
	}
	if (spaceSizes.size() % 2) //if odd number
		avgSpaceSize = spaceSizes[(spaceSizes.size() / 2) + 1];
	else {
		avgSpaceSize = (spaceSizes[spaceSizes.size() / 2] + spaceSizes[spaceSizes.size() / 2 + 1]) / 2;
	}
	printf("Median space size: %d \n", avgSpaceSize);
	for (int i = 0; i < spaces.size(); i++) { //combining small spaces close together
		int lastSpace = max(i - 1, 0);
		if (spaces[i].size <= round(.2 * avgSpaceSize) && abs(spaces[i].start - (spaces[lastSpace].start + spaces[lastSpace].size)) <= round(.3 * avgSpaceSize)) {
			spaces[lastSpace].size = spaces[i].start + spaces[i].size - spaces[lastSpace].start;
			spaces[i] = { -1, -1 };
		}
	}
	if (spaces.size() == 0) printf("no spaces!");
	int lastSpaceSize = 0;
	Space equalSpacing{ -1, -1 };
	for (int i = 0; i < spaces.size(); i++) {
		int lastSpace = max(i - 1, 0);
		Space s = spaces[i];
		if (s.size == -1 || s.start == -1) continue;
		if (abs(s.size - avgSpaceSize) <= round(.5 * avgSpaceSize)) {
			if (equalSpacing.size != -1/* && abs(s.size - lastSpaceSize) < .1 * lastSpaceSize*/) {
				equalSpacing.size = s.start + s.size - equalSpacing.start;
				lastSpaceSize = s.size;
			}
			else if (equalSpacing.size == -1) {
				equalSpacing.start = spaces[lastSpace].start + spaces[lastSpace].size;
				equalSpacing.size = s.start + s.size - equalSpacing.start;
				lastSpaceSize = s.size;
			}
		}
		else {
			if (s.size - avgSpaceSize > round(.5 * avgSpaceSize) && equalSpacing.size != -1) {
				equalSpacing.size = s.start - equalSpacing.start;
				break;
			}
		}
	}
	printf("Equal spacing from: %d to %d \n", equalSpacing.start, equalSpacing.start + equalSpacing.size);
	for (int i = 0; i < img->getHeight(); i++) {
		img->setPixel(equalSpacing.start, i, { 255, 0, 0 });
		img->setPixel(equalSpacing.start + equalSpacing.size, i, { 255, 0, 0 });
	}
#pragma endregion
#pragma region horzSpacing
	int * horzAccumulator = new int[img->getHeight()];
	memset(horzAccumulator, 0, img->getHeight() * sizeof(int));
	std::vector<Space> horzSpaces;
	std::vector<int> horzSpaceSizes;
	for (int i = 0; i < img->getHeight(); i++) {
		for(int x = 0; x < img->getWidth(); x++)
			horzAccumulator[i] += img->getPixel(x, i).avg();
	}
	lastDark = 0;
	for (int i = 0; i <= img->getHeight(); i++) {
		if (i == img->getHeight()) {
			horzSpaces.push_back({ lastDark + 1, i - lastDark });
			horzSpaceSizes.push_back(i - lastDark);
			break;
		}
		if (horzAccumulator[i] / (float)img->getWidth() < 235) {
			if (i - lastDark > 1) {
				horzSpaces.push_back({ lastDark + 1, i - lastDark });
				horzSpaceSizes.push_back(i - lastDark);
			}
			lastDark = i;
		}
	}
	avgSpaceSize = 0;
	for (int i = 0; i < horzSpaceSizes.size(); i++) {
		for (int j = 0; j < horzSpaceSizes.size(); j++) {
			if (horzSpaceSizes[j] < horzSpaceSizes[i]) {
				int size = horzSpaceSizes[i];
				horzSpaceSizes[i] = horzSpaceSizes[j];
				horzSpaceSizes[j] = size;
			}
		}
	}
	if (horzSpaceSizes.size() % 2) //if odd number
		avgSpaceSize = horzSpaceSizes[(horzSpaceSizes.size() / 2) + 1];
	else {
		avgSpaceSize = (horzSpaceSizes[horzSpaceSizes.size() / 2] + horzSpaceSizes[horzSpaceSizes.size() / 2 + 1]) / 2;
	}
	printf("Median HorzSpace size: %d \n", avgSpaceSize);
	for (int i = 0; i < horzSpaces.size(); i++) {
		int lastSpace = max(i - 1, 0);
		if (horzSpaces[i].size <= round(.2 * avgSpaceSize) && abs(horzSpaces[i].start - (horzSpaces[lastSpace].start + horzSpaces[lastSpace].size)) <= round(.3 * avgSpaceSize)) {
			horzSpaces[lastSpace].size = horzSpaces[i].start + horzSpaces[i].size - horzSpaces[lastSpace].start;
			spaces[i] = { -1, -1 };
		}
	}
/*	for (Space s : horzSpaces) {
		printf("Horz Space Size: %d \n", s.size);
		for (int i = 0; i < img->getWidth(); i++) {
			for (int y = s.start; y < s.start + s.size; y++) {
				img->setPixel(i, y, { 0, 0, 255 });
			}
		}
	}
	for (Space s : spaces) {
		for (int i = 0; i < img->getHeight(); i++) {
			for (int x = s.start; x < s.start + s.size; x++) {
				img->setPixel(x, i, { 0, 0, 255 });
			}
		}
	}*/
	if (horzSpaces.size() == 0) printf("no spaces!");
	lastSpaceSize = 0;
	Space horzEqualSpacing{ -1, -1 };
	for (int i = 0; i < horzSpaces.size(); i++) {
		int lastSpace = max(i - 1, 0);
		Space s = horzSpaces[i];
		if (s.size == -1 || s.start == -1) continue;
		if (abs(s.size - avgSpaceSize) <= round(.5 * avgSpaceSize)) {
			if (horzEqualSpacing.size != -1/* && abs(s.size - lastSpaceSize) < .1 * lastSpaceSize*/) {
				horzEqualSpacing.size = s.start + s.size - horzEqualSpacing.start;
				lastSpaceSize = s.size;
			}
			else if (horzEqualSpacing.size == -1) {
				horzEqualSpacing.start = horzSpaces[lastSpace].start + horzSpaces[lastSpace].size;
				horzEqualSpacing.size = s.start + s.size - horzEqualSpacing.start;
				lastSpaceSize = s.size;
			}
		}
		else {
			printf("Stopped bc %d size \n", s.size);
			if (s.size - avgSpaceSize > round(.5 * avgSpaceSize) && horzEqualSpacing.size != -1) {
				horzEqualSpacing.size = s.start - horzEqualSpacing.start;
				break;
			}
		}
	}
	for (int i = 0; i < img->getWidth(); i++) {
		img->setPixel(i, horzEqualSpacing.start, { 255, 0, 0 });
		img->setPixel(i, horzEqualSpacing.start + horzEqualSpacing.size, { 255, 0, 0 });
	}
#pragma endregion
	std::vector<Square> characters;
	for (int x = 0; x < spaces.size(); x++) {
		if (spaces[x].start + spaces[x].size >= equalSpacing.start && spaces[x].start <= equalSpacing.start + equalSpacing.size) {
			for (int y = 0; y < horzSpaces.size(); y++) {
				if (horzSpaces[y].start + horzSpaces[y].size >= horzEqualSpacing.start && horzSpaces[y].start <= horzEqualSpacing.start + horzEqualSpacing.size) {
					Square sq;
					int prevY = max(y - 1, 0);
					int nextX = min(x + 1, spaces.size() - 1);
					int nextY = min(y + 1, horzSpaces.size() - 1);
					sq.x = spaces[x].start + spaces[x].size;
					sq.y = horzSpaces[y].start + horzSpaces[y].size;
					sq.width = spaces[nextX].start - (spaces[x].start + spaces[x].size);
					sq.height = horzSpaces[nextY].start - (horzSpaces[y].start + horzSpaces[y].size);
					characters.push_back(sq);
				}
			}
		}
	}
	for (Square s : characters) {
		for (int i = s.y; i < s.y + s.height; i++) {
			img->setPixel(s.x, i, { 0, 255, 0 });
			img->setPixel(s.x + s.width, i, { 0, 255, 0 });
		}
		for (int i = s.x; i < s.x + s.width; i++) {
			img->setPixel(i, s.y, { 0, 255, 0 });
			img->setPixel(i, s.y + s.height, { 0, 255, 0 });
		}
	}
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] accumulator;
	delete[] horzAccumulator;
	return characters;
}
char * identifyLetters(Image * img, std::vector<Square> locations)
{
/*	char * characterMap = new char[locations.size()];
	Image ** letters = new Image *[26];
	for (int i = ID_A; i < ID_ALPHA_MAX; i++) {
		gui::Resource res = gui::GUI::loadResource(i, LETTER);
		letters[i - ID_A] = new Image(res);
	}
	for (int i = 0; i < locations.size(); i++) {
		std::pair<int, int> minDifference = std::make_pair(0, INT_MAX);
		Image * letter = Image::newImgFrom(img, locations[i].x, locations[i].y, locations[i].width, locations[i].height);
		letter->scaleTo(5, 5);
		for (int j = 0; j < 26; j++) {
			if (abs(letter->integralImageValue(4, 4) - letters[j]->integralImageValue(4, 4)) < minDifference.second) {
				minDifference.first = j;
				minDifference.second = abs(letter->integralImageValue(4, 4) - letters[j]->integralImageValue(4, 4));
			}
		}
		characterMap[i] = minDifference.first + 65; //65 is A in ASCII
		delete letter;
	}
	for (int i = ID_A; i < ID_ALPHA_MAX; i++)
		delete letters[i - ID_A];
	delete[] letters;
	return characterMap;*/
	//Testing...
	Image * letter = Image::newImgFrom(img, locations[1].x, locations[1].y, locations[1].width, locations[1].height);
//	letter->scaleTo(5, 5);
	letter->saveBmp("letterScaled.bmp");
//	delete letter;
	return nullptr;

	//end testing
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
	int diagnol = ceil(sqrt(img->getWidth() * img->getWidth() + img->getHeight() * img->getHeight()));
	Color * buffer = new Color[diagnol * diagnol];
	//Debugging
	for (int i = 0; i < diagnol * diagnol; i++) {
//		buffer[i] = Color{ 0, 255, 0 };
		buffer[i] = Color{ 255, 255, 255 };
	}
	//end debugging
	//mapping from destination back to source and picks out source pixel to reduce the "holes" in the image
	float rotationMatrix[] = {
		cos(radians(-theta)),  -sin(radians(-theta)),
		sin(radians(-theta)),  cos(radians(-theta))
	};
	for (int i = 0; i < diagnol * diagnol; i++) {
		int x = i % diagnol;
		int y = i / diagnol;
		x -= (diagnol - img->getWidth()) / 2;
		y -= (diagnol - img->getWidth()) / 2;
		POINT rtPt = { x - origin.x, y - origin.y };
		POINT rotated = matrixMultiply(rotationMatrix, rtPt);
		rotated.x += origin.x;
		rotated.y += origin.y;
		if (rotated.x > 0 && rotated.x < img->getWidth() && rotated.y > 0 && rotated.y < img->getHeight())
			buffer[i] = img->getPixel(rotated.x, rotated.y);

	}
/*	for (int i = 0; i < img->getWidth() * img->getHeight(); i++) {
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
/*	for (int i = 0; i < diagnol * diagnol; i++) {
		if (buffer[i].g > 250) {
			int x = i % img->getWidth();
			int y = i / img->getWidth();
			int xm = max(0, x - 1);
			int xp = min(img->getWidth(), x + 1);
			int ym = max(0, y - 1);
			int yp = min(img->getHeight() - 1, y + 1);
			Pixel q1{ xm, ym, img->getPixel(xm, ym) };
			Pixel q2{ xp, ym, img->getPixel(xp, ym) };
			Pixel q3{ xm, yp, img->getPixel(xm, yp) };
			Pixel q4{ xp, yp, img->getPixel(xm, yp) };
//			buffer[i] = bilinearInterpolation(q1, q2, q3, q4, { x, y });
		}
	}*/
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
Color bilinearInterpolation(Pixel q1, Pixel q2, Pixel q3, Pixel q4, POINT x)
{
	Color c;
	double x1 = q1.x, x2 = q2.x, y1 = q3.y, y2 = q1.y;
	double r1 = ((x2 - x.x) / (x2 - x1)) * q3.c.r + ((x.x - x1) / (x2 - x1)) * q4.c.r;
	double r2 = ((x2 - x.x) / (x2 - x1)) * q1.c.r + ((x.x - x1) / (x2 - x1)) * q2.c.r;
	c.r = ((y2 - x.y) / (y2 - y1)) * r1 + ((x.y - y1) / (y2 - y1)) * r2;

	r1 = ((x2 - x.x) / (x2 - x1)) * q3.c.g + ((x.x - x1) / (x2 - x1)) * q4.c.g;
	r2 = ((x2 - x.x) / (x2 - x1)) * q1.c.g + ((x.x - x1) / (x2 - x1)) * q2.c.g;
	c.g = ((y2 - x.y) / (y2 - y1)) * r1 + ((x.y - y1) / (y2 - y1)) * r2;

	r1 = ((x2 - x.x) / (x2 - x1)) * q3.c.b + ((x.x - x1) / (x2 - x1)) * q4.c.b;
	r2 = ((x2 - x.x) / (x2 - x1)) * q1.c.b + ((x.x - x1) / (x2 - x1)) * q2.c.b;
	c.b = ((y2 - x.y) / (y2 - y1)) * r1 + ((x.y - y1) / (y2 - y1)) * r2;
	return c;
}
Rect::operator Square()
{
	return{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}
