#pragma comment(lib, "GUI API.lib")
#include <Image.h>
bool isFile(char * path) {
	struct stat fs;
	if (stat(path, &fs) == 0) {
		if (fs.st_mode & S_IFDIR || fs.st_mode & S_IFREG)
			return true;
	}
	return false;
}
bool isBmp(char * path) {
	char * pointer = path;
	char ending[] = ".bmp";
	int match = 0;
	while (*pointer != '\0') {
		if (tolower(*pointer) == ending[match])
			match++;
		else match = 0;
		pointer++;
	}
	return match == sizeof(ending) - 1 ? true : false;
}
int main() {
	WIN32_FIND_DATA fSearch;
	ZeroMemory(&fSearch, sizeof(fSearch));
	char dir[] = "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\HelperProgram\\arial";
	char path[MAX_PATH];
	sprintf_s(path, MAX_PATH, "%s%s", dir, "\\*");
	HANDLE handle = FindFirstFile(path, &fSearch);
	gui::GUI::bindWindow(NULL);
	while(handle != INVALID_HANDLE_VALUE){
		sprintf_s(path, MAX_PATH, "%s\\%s", dir, fSearch.cFileName);
		char saveName[MAX_PATH];
		if (isFile(path) && isBmp(path)) {
			printf("%s \n", path);
			Image * img = new Image(path);
			img->toMonochrome();
			img->scaleTo(5, 5);
			sprintf_s(saveName, MAX_PATH, "C:\\Users\\stephen\\Documents\\Visual Studio 2015\\Projects\\Puzzle Solver + GDI API\\HelperProgram\\saves\\%c ar.bmp", fSearch.cFileName[0]);
			img->saveBmp(saveName);
			delete img;
		}
		if(FindNextFile(handle, &fSearch) == FALSE) break;
	}
	FindClose(handle);
/*	Image * img = new Image("mpAlphabet.bmp");
	int startChar = -1;
	int endChar = -1;
	char saves = 'a';
	const int threshold = img->integralImageValue(img->getWidth() - 1, img->getHeight() - 1) / (img->getWidth() * img->getHeight());
	for (int i = 0; i < img->getWidth(); i++) {
		int accumulator = 0;
		for (int j = 0; j < img->getHeight(); j++) {
			accumulator += img->getPixel(i, j).avg() < threshold ? 1 : 0;
		}
		if (startChar == -1 && accumulator >= 1)
			startChar = i;
		else if (accumulator >= 1)
			endChar = i;
		else if (accumulator < 1 && endChar != -1) {
			int height = 0;
			for (int j = startChar; j <= endChar; j++) {
				for (int k = 0; k < img->getHeight(); k++) {
					if (img->getPixel(j, k).avg() <= threshold)
						height = max(height, k);
				}
			}
			ImgPtr out = new Image(endChar - startChar + 2, img->getHeight());
			out->clearBmp(255);
			char saveFile[MAX_PATH];
			sprintf_s(saveFile, MAX_PATH, "%c mp.bmp", saves++);
			for (int x = 0; x < out->getWidth(); x++) {
				for (int y = 0; y < img->getHeight(); y++) {
					if(startChar + x - 1 >= 0)
						out->setPixel(x, y, img->getPixel(startChar + x - 1, y));
				}
			}

			out->scaleTo(5, 5);
			out->saveBmp(saveFile);
			startChar = -1;
			endChar = -1;
		}
	}*/
	getchar();
}