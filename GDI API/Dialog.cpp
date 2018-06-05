#include "Dialog.h"

gui::fileFilter::fileFilter(char * name, std::initializer_list<std::string> typeList) : nameOfFilter(name), filter("")
{
	int i = 1;
	for (auto it = typeList.begin(); it != typeList.end(); it++) {
		if(i == typeList.size())
			filter += '*' + (*it);
		else
			filter += '*' + (*it) + ';';
		i++;
	}
}

void gui::fileFilter::addFileType(std::string type)
{
	filter += '*' + type + ';';
}
std::string gui::openFileDialog(fileFilter * filter, HWND parentWindow, int params)
{
	char path[MAX_PATH];
	OPENFILENAME op;
	ZeroMemory(path, MAX_PATH);
	ZeroMemory(&op, sizeof(OPENFILENAME));
	op.lStructSize = sizeof(OPENFILENAME);
	op.hwndOwner = parentWindow;
	if (filter != nullptr)
		op.lpstrFilter = filter->getRule().c_str();
	op.lpstrFile = path;
	op.nMaxFile = MAX_PATH;
	op.lpstrTitle = "Open";
	op.Flags = params;
	if (GetOpenFileName(&op)) {
		return std::string(path);
	}
	else if(CommDlgExtendedError() != 0)
		MessageBox(parentWindow, "Error opening file!", "Open Dialog", MB_OK | MB_ICONERROR);
	return std::string("");

}
std::string gui::saveFileDialog(fileFilter * filter, HWND parentWindow, int params)
{
	char path[MAX_PATH];
	OPENFILENAME op;
	ZeroMemory(path, MAX_PATH);
	ZeroMemory(&op, sizeof(OPENFILENAME));
	op.lStructSize = sizeof(OPENFILENAME);
	op.hwndOwner = parentWindow;
	if (filter != nullptr)
		op.lpstrFilter = filter->getRule().c_str();
	op.lpstrFile = path;
	op.nMaxFile = MAX_PATH;
	op.lpstrTitle = "Save";
	op.Flags = params;
	if (GetSaveFileName(&op)) {
		return std::string(path);
	}
	else if(CommDlgExtendedError() != 0)
		MessageBox(parentWindow, "Error saving file!", "Save Dialog", MB_OK | MB_ICONERROR);
	return std::string("");
}
