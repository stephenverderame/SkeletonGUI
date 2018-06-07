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

gui::Resource gui::GUI::loadResource(int file, int type)
{
	Resource res = {};
	HMODULE handle = GetModuleHandle(NULL);
	HRSRC rc = FindResource(handle, MAKEINTRESOURCE(file), MAKEINTRESOURCE(type));
	HGLOBAL rcData = LoadResource(handle, rc);
	res.size = SizeofResource(handle, rc);
	if (res.data != nullptr) delete[] res.data;
	res.data = new char[res.size + 1];
	const char * cpy = (char *)LockResource(rcData);
	memcpy(res.data, cpy, res.size);
	res.data[res.size] = '\0';
	return res;

}
