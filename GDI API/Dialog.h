#pragma once
#include <Windows.h>
#include <string>
#include <initializer_list>
namespace gui {
	class fileFilter {
	private:
		std::string filter;
		std::string nameOfFilter;
	public:
		fileFilter() : filter(""), nameOfFilter("") {};
		fileFilter(char * name, std::initializer_list<std::string> typeList);
		void setName(char * name) { nameOfFilter = name; }
		void addFileType(std::string type);
		std::string getRule() { return nameOfFilter + '\0' + filter + '\0'; }
	};
	std::string openFileDialog(fileFilter * filter = nullptr, HWND parentWindow = NULL, int params = OFN_FILEMUSTEXIST);
	std::string saveFileDialog(fileFilter * filter = nullptr, HWND parentWindow = NULL, int params = NULL);
}