#include "OCR.h"
POINT matrixMultiply(float * matrix, POINT vector) {
	POINT result = { 0, 0 };
	result.x = matrix[0] * vector.x + matrix[1] * vector.y;
	result.y = matrix[2] * vector.x + matrix[3] * vector.y;
	return result;
}
float findSkewAngle(Image * img, POINT * origin, Bounds * skewBounds) {
	if (origin == nullptr)
		*origin = getOrigin(img);
	int startX = origin->x;
	int startY = origin->y;
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
		if(skewBounds != NULL)
			if (i < floor(skewBounds->min) || i > ceil(skewBounds->max)) continue;
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
/*	for (int i = 0; i < diagnol; i++) {
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
			img->setPixel(drawPoint.x, drawPoint.y, Color{ 0, 0, 255 });
		}
	}*/
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
/*	printf("Equal spacing from: %d to %d \n", equalSpacing.start, equalSpacing.start + equalSpacing.size);
	for (int i = 0; i < img->getHeight(); i++) {
		img->setPixel(equalSpacing.start, i, { 255, 0, 0 });
		img->setPixel(equalSpacing.start + equalSpacing.size, i, { 255, 0, 0 });
	}*/
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
/*	for (Space s : spaces) {
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
/*	for (int i = 0; i < img->getWidth(); i++) {
		img->setPixel(i, horzEqualSpacing.start, { 255, 0, 0 });
		img->setPixel(i, horzEqualSpacing.start + horzEqualSpacing.size, { 255, 0, 0 });
	}*/
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
					if (sq.width < 3 || sq.height < 3) continue;
					if (sq.y > horzEqualSpacing.start + horzEqualSpacing.size) continue;
					if (sq.x > equalSpacing.start + equalSpacing.size) continue;
					int upY = sq.y;
					int downY = sq.y + sq.height;
					int leftX = sq.x;
					int rightX = sq.x + sq.width;
					bool done[4] = { false, false, false, false };
					while (!(done[0] && done[1] && done[2] && done[3])) {
						int upAcc = 0, downAcc = 0;
						for (int i = -1; i < sq.width + 1; i++) {
							upAcc += img->getPixel(sq.x + i, upY).avg() < 180 ? 1 : 0;
							downAcc += img->getPixel(sq.x + i, downY).avg() < 180 ? 1 : 0;
						}
						if (upAcc > 1 && upY <= sq.y) upY--;
//						else if (upY >= sq.y && upAcc <= 1) upY++;
						else done[0] = true;
						if (downAcc > 1 && downY >= sq.y + sq.height) downY++;
//						else if (downY <= sq.y + sq.height && downAcc <= 1) downY--;
						else done[1] = true;

						int leftAcc = 0, rightAcc = 0;
						for (int i = -1; i < sq.height + 1; i++) {
							leftAcc += img->getPixel(leftX, sq.y + i).avg() < 180 ? 1 : 0;
							rightAcc += img->getPixel(rightX, sq.y + i).avg() < 180 ? 1 : 0;
						}
						if (leftAcc > 1 && leftX <= sq.x) leftX--;
//						else if (leftAcc <= 1 && leftX >= sq.x) leftX++;
						else done[2] = true;
						if (rightAcc > 1 && rightX >= sq.x + sq.width) rightX++;
//						else if (rightAcc <= 1 && rightX <= sq.x + sq.width) rightX--;
						else done[3] = true;
						
					}
//					Square old = sq;					
/*					sq.y = changed[1] ? topY : upY;
					sq.x = changed[2] ? boundLeft : leftX;
					sq.height = changed[0] ? bottomY - sq.y : downY - sq.y;
					sq.width = changed[3] ? boundRight - sq.x : rightX - sq.x; */
					sq.y = upY;
					sq.height = downY - sq.y;
					sq.x = leftX;
					sq.width = rightX - sq.x;					
					if (sq.width < 3 || sq.height < 3) continue;
					if (sq.y > horzEqualSpacing.start + horzEqualSpacing.size) continue;
					characters.push_back(sq);
				}
			}
		}
	}
/*	for (Square s : characters) {
		for (int i = s.y; i < s.y + s.height; i++) {
			img->setPixel(s.x, i, { 0, 255, 0 });
			img->setPixel(s.x + s.width, i, { 0, 255, 0 });
		}
		for (int i = s.x; i < s.x + s.width; i++) {
			img->setPixel(i, s.y, { 0, 255, 0 });
			img->setPixel(i, s.y + s.height, { 0, 255, 0 });
		}
	}*/
	RECT r;
	GetClientRect(gui::GUI::useWindow(), &r);
	InvalidateRect(gui::GUI::useWindow(), &r, TRUE);
	delete[] accumulator;
	delete[] horzAccumulator;
	return characters;
}
#define SAMPLE_WIDTH 5
#define SAMPLE_HEIGHT 5
bool isBmp(char * path) {
	char * pointer = path;
	char ending[] = ".bmp";
	int match = 0;
	while (*pointer != '\0') {
		if (*pointer == ending[match])
			match++;
		else match = 0;
		pointer++;
	}
	return match == sizeof(ending) - 1 ? true : false;
}
SearchGrid identifyLetters(Image * img, std::vector<Square> locations)
{
	SearchGrid grid;
	std::vector<KnownSample *> letters;
/*	for (int i = 0; i < 26; i++) {
//		gui::Resource res = gui::GUI::loadResource(i, LETTER);
		std::stringstream ss;
		ss << "letters2\\" << (char)(i + 65) << ".bmp";
		Image * letterImage = new Image(ss.str().c_str());
//		if(i + 65 == 'n' || i + 65 == 'N') letterImage->saveBmp("testKnown.bmp"); 
		letters.push_back(letterImage);
//		printf("I: %d \n", i);
	}*/
	WIN32_FIND_DATA fData;
	HANDLE hand = FindFirstFile("C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\Puzzle Solver + GDI API\\letters\\*", &fData);
	char fileRead[MAX_PATH];
	while (hand != INVALID_HANDLE_VALUE) {		
		if (isBmp(fData.cFileName)) {
			printf("%s \n", fData.cFileName);
			sprintf_s(fileRead, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\Puzzle Solver + GDI API\\letters\\%s", fData.cFileName);
			if (!isalpha(fData.cFileName[0])) printf("img name is not a letter! \n");
			letters.push_back(new KnownSample{ new Image(fileRead), (char)toupper(fData.cFileName[0]) });			
		}
		if (FindNextFile(hand, &fData) == FALSE) break;
	}
	struct stat fInfo;
	if (stat("data.ml", &fInfo) == 0) {
		std::ifstream file;
		file.open("data.ml", std::ios::in | std::ios::binary);
		if(file.is_open()){
			int size = 0;
			file.read((char*)&size, sizeof(int));
			printf("Size: %d \n", size);
			channel * data = new channel[26 * size];
			file.read((char*)data, 26 * size);
			for (int i = 0; i < size; i++) {
				channel * dr = data + (26 * i);
				Image * imgM = new Image(5, 5);
				char character = dr[25];
				int sum = 0;
				for (int j = 0; j < 25; j++) {
					int x = j % 5;
					int y = j / 5;
					imgM->setPixel(x, y, { dr[j], dr[j], dr[j] });
					sum += dr[j];
				}
				letters.push_back(new KnownSample{ imgM, (char)toupper(character) });
				printf("Read %c with %d \n", character, sum);
			}
			file.close();
			delete data;
		}
	}
	std::multimap<char, channel *> guaranteed;
	for (int i = 0; i < locations.size(); i++) {
		std::pair<char, int> minDifference = std::make_pair(0, INT_MAX);
		Image * letter = new Image("white.bmp");
		letter->resize(locations[i].width, locations[i].height);
		for (int x = 0; x < locations[i].width; x++) {
			for (int y = 0; y < locations[i].height; y++) {
				letter->setPixel(x, y, img->getPixel(x + locations[i].x, y + locations[i].y));
			}
		}
		if (i == 4) letter->saveBmp("testUnknownPreScale.bmp");
		letter->scaleTo(SAMPLE_WIDTH, SAMPLE_HEIGHT);
		if (i == 4) letter->saveBmp("testUnknown.bmp");
		for (int j = 0; j < letters.size(); j++) {
			float diffScore = 0;		
			int blackL = 0;
			int blackK = 0;
			float ratio1L, ratio2L, ratio1K, ratio2K;
			int threshL = letter->integralImageValue(letter->getWidth() - 1, letter->getHeight() - 1) / (float)(SAMPLE_WIDTH * SAMPLE_HEIGHT);
			int threshK = (*letters[j])->integralImageValue((*letters[j])->getWidth() - 1, (*letters[j])->getHeight() - 1) / (float)(SAMPLE_WIDTH * SAMPLE_HEIGHT);
			int sqSize = (SAMPLE_WIDTH * SAMPLE_HEIGHT) / 25;
			int sqWidth = sqrt(sqSize);
			for (int k = 0; k < 25; k++) {
				int x = k % 5;
				int y = k / 5;
				float lquadScore = 0;
				float kquadScore = 0;
				for (int l = 0; l < sqSize; l++) {
					int x1 = l % sqWidth;
					int y1 = l / sqWidth;
					lquadScore += letter->getPixel(x * sqWidth + x1, y * sqWidth + y1).avg();
					kquadScore += (*letters[j])->getPixel(x * sqWidth + x1, y * sqWidth + y1).avg();

				}
				lquadScore /= sqSize;
				kquadScore /= sqSize;
				diffScore += (kquadScore - lquadScore) * (kquadScore - lquadScore);
				
			}
			for (int k = 0; k < SAMPLE_WIDTH * SAMPLE_HEIGHT; k++) {
				int x = k % SAMPLE_WIDTH;
				int y = k / SAMPLE_WIDTH;
				if (k == floor((SAMPLE_WIDTH * SAMPLE_HEIGHT) / 2.0)) {
					ratio1L = blackL / (float)(floor((SAMPLE_WIDTH * SAMPLE_HEIGHT) / 2.0) - blackL);
					ratio1K = blackK / (float)(floor((SAMPLE_WIDTH * SAMPLE_HEIGHT) / 2.0) - blackK);
					blackL = 0;
					blackK = 0;
				}
				else if (k == SAMPLE_WIDTH * SAMPLE_HEIGHT - 1) {
					ratio2L = blackL / (float)(ceil((SAMPLE_WIDTH * SAMPLE_HEIGHT) / 2.0) - blackL);
					ratio2K = blackK / (float)(ceil((SAMPLE_WIDTH * SAMPLE_HEIGHT) / 2.0) - blackK);
				}
				blackL += letter->getPixel(x, y).avg() < threshL ? 1 : 0;
				blackK += (*letters[j])->getPixel(x, y).avg() < threshK ? 1 : 0;
			}
			std::vector<double> widthL, heightL, widthK, heightK;
			for (int k = 0; k < SAMPLE_WIDTH; k++) {
				widthL.push_back(letter->getPixel(round(SAMPLE_HEIGHT / 2.0), k).avg() < threshL ? 1 : 0);
				heightL.push_back(letter->getPixel(k, round(SAMPLE_WIDTH / 2.0)).avg() < threshL ? 1 : 0);
				widthK.push_back((*letters[j])->getPixel(round(SAMPLE_HEIGHT / 2.0), k).avg() < threshL ? 1 : 0);
				heightK.push_back((*letters[j])->getPixel(k, round(SAMPLE_WIDTH / 2.0)).avg() < threshL ? 1 : 0);
			}
			int mins[] =  { INT_MAX, INT_MAX, INT_MAX, INT_MAX };
			int maxes[] = { INT_MIN, INT_MIN, INT_MIN, INT_MIN };
			for (int k = 0; k < SAMPLE_WIDTH; k++) {
				if (widthL[k]) {
					min(mins[0], k);
					max(maxes[0], k);
				}
				if (widthK[k]) {
					min(mins[1], k);
					max(maxes[1], k);
				}
				if (heightL[k]) {
					min(mins[2], k);
					max(maxes[2], k);
				}
				if (heightK[k]) {
					min(mins[3], k);
					max(maxes[3], k);
				}
			}
			diffScore += (ratio1L - ratio1K) * (ratio1L - ratio1K) + (ratio2L - ratio2K) * (ratio2L - ratio2K);
			diffScore += (((maxes[0] - mins[0]) / (maxes[2] - mins[2])) - ((maxes[1] - mins[1]) / (maxes[3] - mins[3])) * (((maxes[0] - mins[0]) / (maxes[2] - mins[2])) - ((maxes[1] - mins[1]) / (maxes[3] - mins[3]))));
			if (diffScore < minDifference.second) {
				minDifference.first = letters[j]->letter;
				minDifference.second = diffScore;
			}
		}
		grid.addLetter(minDifference.first, locations[i].x, locations[i].y);
		if (minDifference.second < 81000 && minDifference.second > 100) {
			//almost gaurunteed match
			channel * data = new channel[25];
			int sum = 0;
			for (int i = 0; i < 25; i++) {
				int x = i % 5;
				int y = i / 5;
				data[i] = (channel)letter->getPixel(x, y).avg();
				sum += data[i];
			}
			guaranteed.insert(std::make_pair(minDifference.first, data));
			printf("Insert %c with %d \n", minDifference.first, sum);

		}
		delete letter;
	}
	if (guaranteed.size() > 0) {
		channel * data = new channel[26 * guaranteed.size()];
		int i = 0;
		for (auto it = guaranteed.begin(); it != guaranteed.end(); it++) {
			memcpy_s(data + i * 26, 26 * guaranteed.size(), (*it).second, 25);
			data[i * 26 + 25] = (*it).first;
			delete (*it).second;
			i++;
		}
		struct stat fInfo;
		if (stat("data.ml", &fInfo) == 0) {
			//file exists
			printf("Exists \n");
			std::fstream file;
			file.open("data.ml", std::ios::in | std::ios::out | std::ios::binary);
			if (file.is_open()) {
				int size;
				file.read((char*)&size, sizeof(int));
				size += guaranteed.size();
				file.write((char*)&size, sizeof(int));
				file.seekp(0, std::ios::end);
				file.write((char*)data, 26 * guaranteed.size());
				file.close();
			}
			else printf("Could not open for read/write \n");
		}
		else {
			std::ofstream file;
			file.open("data.ml", std::ios::binary | std::ios::out);
			if (file.is_open()) {
				int size = guaranteed.size();
				printf("Writing size of: %d \n", size);
				file.write((char*)&size, sizeof(int));
				file.write((char*)data, 26 * guaranteed.size());
				file.close();
			}
			else printf("Could not open for writing \n");
		}
	}
	for (int i = 0; i < letters.size(); i++)
		delete letters[i];
	grid.iterateRowbyRow();
	return grid;
 }
 void augmentDataSet(std::vector<Square> locations, std::vector<char> knowns, Image * img, int firstKnown)
 {
	 int size = min(locations.size(), firstKnown + knowns.size());
	 for (int i = firstKnown; i < size; i++) {
		 ImgPtr image = new Image(locations[i].width, locations[i].height);
		 for (int j = 0; j < locations[i].width * locations[i].height; j++) {
			 int x = j % locations[i].width;
			 int y = j / locations[i].width;
			 image->setPixel(x, y, img->getPixel(x + locations[i].x, y + locations[i].y));
		 }
		 char path[MAX_PATH];
		 srand(clock());
		 int id = rand() % 1000;
		 sprintf_s(path, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\Puzzle Solver + GDI API\\letters\\%c %drw.bmp", knowns[i], id);
		 image->scaleTo(5, 5);
		 image->saveBmp(path);
	 }
 }
POINT getOrigin(Image * img)
{
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
			if (xp != (int)(x + (.15 * img->getWidth())))
				xm -= (x + .15 * img->getWidth()) - (img->getWidth() - 1);
			else if (xm != (int)(x - (.15 * img->getWidth())))
				xp += 0 - (x - .15 * img->getWidth());
			if (yp != (int)(y + .15 * img->getHeight()))
				ym -= (y + .15 * img->getHeight()) - (img->getHeight() - 1);
			else if (ym != (int)(y - .15 * img->getHeight()))
				yp += 0 - (y - .15 * img->getHeight());
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
	return { startX, startY };
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
	float rotationMatrix[] = { //rotating to the negative angle is same thing as taking inverse of rotation matrix to that angle
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
//#define THRESHOLD 200
CharacterFeatures getImageScore(Image * img)
{
	int score = 0;
	char elbow1 = 0;
	char elbow2 = 0;
	char dash1 = 0;
	char dash2 = 0;
	char v = 0;
	const int THRESHOLD = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) / (float)(img->getHeight() * img->getWidth());
	for (int i = 0; i < img->getHeight() * img->getWidth(); i++) {
		int x = i % img->getWidth();
		int y = i / img->getWidth();
		for (int j = -1; j <= 1; j++) {
			if (j == 0) continue;
			if (y + j >= 0 && y + j < img->getHeight() && x - j < img->getWidth() && x - j >= 0)
				if (img->getPixel(x, y).avg() < THRESHOLD && img->getPixel(x - j, y).avg() < THRESHOLD && img->getPixel(x, y + j).avg() < THRESHOLD && img->getPixel(x - j, y + j).avg() >= THRESHOLD) elbow1++;
//				elbow1 += img->getPixel(x, y).avg() < THRESHOLD ? (img->getPixel(x - j, y).avg() < THRESHOLD ? (img->getPixel(x, y + j).avg() < THRESHOLD ? (img->getPixel(x - j, y +j).avg() > THRESHOLD ? 1 : 0) : 0) : 0) : 0;
			if (y + j >= 0 && y + j < img->getHeight() && x + j >= 0 && x + j < img->getWidth())
				if (img->getPixel(x, y).avg() < THRESHOLD && img->getPixel(x + j, y).avg() < THRESHOLD && img->getPixel(x, y + j).avg() < THRESHOLD && img->getPixel(x + j, y + j).avg() >= THRESHOLD) elbow2++;
//				elbow2 += img->getPixel(x, y).avg() < THRESHOLD ? (img->getPixel(x + j, y).avg() < THRESHOLD ? (img->getPixel(x, y + j).avg() < THRESHOLD ? (img->getPixel(x + j, y + j).avg() > THRESHOLD ? 1 : 0) : 0) : 0) : 0;
//				dash += img->getPixel(x, y).avg() < THRESHOLD ? (img->getPixel(x + j, y + j).avg() < THRESHOLD ? (img->getPixel(x - j, y - j).avg() < THRESHOLD ? 1 : 0) : 0) : 0;
			if (y + j >= 0 && y + j < img->getHeight() && x + 1 < img->getWidth() && x - 1 >= 0)
				if (img->getPixel(x, y).avg() < THRESHOLD && img->getPixel(x + 1, y + j).avg() < THRESHOLD && img->getPixel(x - 1, y + j).avg() < THRESHOLD && img->getPixel(x, y + j).avg() >= THRESHOLD) v++;
//				v += img->getPixel(x, y).avg() < THRESHOLD ? (img->getPixel(x + 1, y + j).avg() < THRESHOLD ? (img->getPixel(x - 1, y + j).avg() < THRESHOLD ? 1 : 0) : 0) : 0;

		}
		if (y + 1 >= 0 && y + 1 < img->getHeight() && x + 1 >= 0 && x + 1 < img->getWidth() && x - 1 >= 0 && x - 1 < img->getWidth() && y - 1 >= 0 && y - 1 < img->getHeight()) {
			if (img->getPixel(x, y).avg() < THRESHOLD && img->getPixel(x + 1, y + 1).avg() < THRESHOLD && img->getPixel(x - 1, y - 1).avg() < THRESHOLD) dash1++;
			if (img->getPixel(x, y).avg() < THRESHOLD && img->getPixel(x + 1, y - 1).avg() < THRESHOLD && img->getPixel(x - 1, y + 1).avg() < THRESHOLD) dash2++;
		}

		
	}
	return CharacterFeatures{elbow1, elbow2, dash1, dash2, v, THRESHOLD};
}
Rect::operator Square()
{
	return{ topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y };
}

void SearchGrid::addLetter(char c, int x, int y)
{
	if (letters.size() < 1) {
		row0Y = y;
		column0X = x;
		lastRow = std::make_pair(0, y);
		lastColumn = std::make_pair(0, x);
		letters.push_back(new Letter{ 0, 0, c });
	}
	else {
		int currentRow;
		int currentColumn;
		if (abs(lastRow.second - y) < 5)
			currentRow = lastRow.first;
		else if (y > lastRow.second) {
			//this row is further down
			lastRow = std::make_pair(lastRow.first + 1, y);
			currentRow = lastRow.first;
			maxRows = max(maxRows, currentRow);
		}
		else if (y < lastRow.second) {
			if (abs(row0Y - y) < 5) {
				lastRow = std::make_pair(0, row0Y);
				currentRow = 0;
//				printf("Row reset \n");
			}
			else
				printf("IDK how to handle this row \n");
		}

		if (abs(lastColumn.second - x) < 5) {
			currentColumn = lastColumn.first;
//			printf("Current column \n");
		}
		else if (x > lastColumn.second) {
			lastColumn = std::make_pair(lastColumn.first + 1, x);
			currentColumn = lastColumn.first;
			maxColumns = max(maxColumns, currentColumn);
//			printf("Column greater: %d \n", currentColumn);
		}
		else if (x < lastColumn.second) {
//			if (abs(x - column0X) < 5) {
				lastColumn = std::make_pair(0, column0X);
				currentColumn = 0;
//				printf("Column reset \n");
//			}
//			else
//				printf("IDK column \n");
		}
//		printf("%d %d \n", currentRow, currentColumn);
		letters.push_back(new Letter{ currentRow, currentColumn, c });
	}
}

SearchGrid::~SearchGrid()
{
	for (int i = 0; i < letters.size(); i++) {
		delete letters[i];
	}
}

void SearchGrid::iterateRowbyRow()
{
	printf("Max rows: %d   Max columns: %d \n", maxRows, maxColumns);
	for (int r = 0; r <= maxRows; r++) {
		for (int c = 0; c <= maxColumns; c++) {
			for (Letter * l : letters) {
				if (l->row == r && l->column == c)
					printf("%c ", l->letter);
			}
		}
		printf("\n");
	}
}
