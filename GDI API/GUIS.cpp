#include "GUIS.h"
HWND gui::GUI::window;
int gui::GUI::_height;
int gui::GUI::_width;

std::string loadFromTextFileToWin32ControlTextFormat(char * file) {
	std::ifstream input;
	input.open(file);
	std::string buffer = "";
	std::string fileText = "";
	if (input.is_open()) {
		while (std::getline(input, buffer)) {
			fileText += buffer += "\r\n";
		}
		input.close();
	}
	return fileText;
}

gui::Resource gui::GUI::loadResource(int file, int type, HMODULE handle)
{
	Resource res = {};
	HRSRC rc = FindResource(handle, MAKEINTRESOURCE(file), MAKEINTRESOURCE(type));
	if (rc == NULL) printf("Find resource fail! \n");
	HGLOBAL rcData = LoadResource(handle, rc);
	res.size = SizeofResource(handle, rc);
	const char * cpy = (char *)LockResource(rcData);
	res.data.assign(cpy, res.size);
	printf("Resource data: %s \n", res.data.c_str());
	return res;

}
